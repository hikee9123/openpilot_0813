from cereal import car, log
from common.realtime import DT_CTRL
from common.numpy_fast import clip, interp
from selfdrive.config import Conversions as CV
from selfdrive.car import apply_std_steer_torque_limits
from selfdrive.car.hyundai.hyundaican import create_lkas11, create_clu11, create_lfahda_mfc, create_acc_commands, create_acc_opt, create_frt_radar_opt, create_mdps12, create_hda_mfc, create_scc12
from selfdrive.car.hyundai.values import Buttons, CarControllerParams, CAR, FEATURES
from opendbc.can.packer import CANPacker

from selfdrive.car.hyundai.navicontrol  import NaviControl

VisualAlert = car.CarControl.HUDControl.VisualAlert
LongCtrlState = car.CarControl.Actuators.LongControlState
LaneChangeState = log.LateralPlan.LaneChangeState

import common.loger as trace1

class CarController():
  def __init__(self, dbc_name, CP, VM):
    self.p = CarControllerParams(CP)
    self.packer = CANPacker(dbc_name)

    self.apply_steer_last = 0
    self.car_fingerprint = CP.carFingerprint
    self.steer_rate_limited = False
    self.last_resume_frame = 0
    self.accel = 0

    self.resume_cnt = 0
    self.last_lead_distance = 0
    self.lkas11_cnt = 0
    self.scc_live = 0
    self.NC = NaviControl(self.p)
    self.steerWarning_time = 0

    # hud
    self.hud_timer_left = 0
    self.hud_timer_right = 0
    self.steer_timer_apply_torque = 1.0
    self.DT_STEER = 0.01


  def process_hud_alert(self, enabled, c ):
    visual_alert = c.hudControl.visualAlert
    left_lane = c.hudControl.leftLaneVisible
    right_lane = c.hudControl.rightLaneVisible

    sys_warning = (visual_alert in [VisualAlert.steerRequired, VisualAlert.ldw])

    if left_lane:
      self.hud_timer_left = 100

    if right_lane:
      self.hud_timer_right = 100

    if self.hud_timer_left:
      self.hud_timer_left -= 1
 
    if self.hud_timer_right:
      self.hud_timer_right -= 1


    # initialize to no line visible
    sys_state = 1
    if self.hud_timer_left and self.hud_timer_right or sys_warning:  # HUD alert only display when LKAS status is active
      if enabled or sys_warning:
        sys_state = 3
      else:
        sys_state = 4
    elif self.hud_timer_left:
      sys_state = 5
    elif self.hud_timer_right:
      sys_state = 6

    return sys_warning, sys_state


  def lkas_active_control( self, enabled, CS, path_plan ):
    steerAngleDegAbs = abs(CS.out.steeringAngleDeg)
    #steeringTorque = abs(CS.out.steeringTorque)
    
    # 1. steer warning
    if CS.out.steerWarning and CS.out.steeringPressed:
      self.steerWarning_time = 100
    elif self.steerWarning_time:
      if steerAngleDegAbs > 10:
        self.steerWarning_time = 100 

    if self.steerWarning_time > 0:
      self.steerWarning_time -= 1

   
    lkas_active = enabled and not self.steerWarning_time and CS.out.vEgo >= CS.CP.minSteerSpeed and CS.out.cruiseState.enabled
    return lkas_active
  
  def smooth_steer( self, apply_torque ):
    if self.steer_timer_apply_torque >= 1:
      return int(round(float(apply_torque)))
    elif self.steer_timer_apply_torque >= 1:
      self.steer_timer_apply_torque = 1
    else:
      self.steer_timer_apply_torque += self.DT_STEER 

    apply_torque *= self.steer_timer_apply_torque

    return  int(round(float(apply_torque)))


  def update_debug(self, CS, c ):
    actuators = c.actuators
    vFuture = c.hudControl.vFuture * 3.6
    str_log1 = 'MODE={:.0f} vF={:.1f} gas={:.2f}'.format( CS.cruise_set_mode, vFuture, CS.out.gas )
    trace1.printf2( '{}'.format( str_log1 ) )


    str_log1 = 'acc={:.2f}, RV={:.2f},  {:.2f}  '.format( actuators.accel, CS.aReqValue, self.accel )
    trace1.printf3( '{}'.format( str_log1 ) )
  
  
  def update_scc12(self, can_sends,  c, CS, frame ):
    actuators = c.actuators
    enabled = c.enabled and CS.out.cruiseState.accActive

    accel = actuators.accel if enabled else 0
    #if accel < 0:
    #  accel = interp(accel - CS.out.aEgo, [-1.0, -0.5], [2 * accel, accel])

    accel = clip(accel, CarControllerParams.ACCEL_MIN, CarControllerParams.ACCEL_MAX)

    if CS.out.vEgo > 0.1 and (CS.aReqValue > accel) and enabled:
      #accel = CS.aReqValue
      can_sends.append( create_scc12(self.packer, accel, enabled, frame, self.scc_live, CS.scc12 ) )

    self.accel = accel
    return can_sends


  def updateLongitudinal(self, can_sends,  c, CS, frame):
    enabled = c.enabled and CS.out.cruiseState.accActive
    actuators = c.actuators
    hud_speed = c.hudControl.setSpeed
    # tester present - w/ no response (keeps radar disabled)
    if (frame % 100) == 0:
        can_sends.append([0x7D0, 0, b"\x02\x3E\x80\x00\x00\x00\x00\x00", 0])

    if frame % 2 == 0:
      lead_visible = False
      accel = actuators.accel if enabled else 0

      jerk = clip(2.0 * (accel - CS.out.aEgo), -12.7, 12.7)

      if accel < 0:
        accel = interp(accel - CS.out.aEgo, [-1.0, -0.5], [2 * accel, accel])

      accel = clip(accel, CarControllerParams.ACCEL_MIN, CarControllerParams.ACCEL_MAX)

      stopping = (actuators.longControlState == LongCtrlState.stopping)
      set_speed_in_units = hud_speed * (CV.MS_TO_MPH if CS.clu11["CF_Clu_SPEED_UNIT"] == 1 else CV.MS_TO_KPH)
      can_sends.extend(create_acc_commands(self.packer, enabled, accel, jerk, int(frame / 2), lead_visible, set_speed_in_units, stopping))
      self.accel = accel

    # 5 Hz ACC options
    if frame % 20 == 0:
      can_sends.extend(create_acc_opt(self.packer))

    # 2 Hz front radar options
    if frame % 50 == 0:
      can_sends.append(create_frt_radar_opt(self.packer))      

    return  can_sends

  def update_resume(self, can_sends,  c, CS, frame, path_plan):
    pcm_cancel_cmd = c.cruiseControl.cancel
    if pcm_cancel_cmd:
      can_sends.append(create_clu11(self.packer, frame, CS.clu11, Buttons.CANCEL))
    elif CS.out.cruiseState.standstill:
      if CS.out.gasPressed:
        self.last_lead_distance = 0      
      # run only first time when the car stopped
      elif self.last_lead_distance == 0:  
        # get the lead distance from the Radar
        self.last_lead_distance = CS.lead_distance
        self.resume_cnt = 0
      # when lead car starts moving, create 6 RES msgs
      elif CS.lead_distance != self.last_lead_distance and (frame - self.last_resume_frame) > 5:
        can_sends.append(create_clu11(self.packer, self.resume_cnt, CS.clu11, Buttons.RES_ACCEL))
        self.resume_cnt += 1
        # interval after 6 msgs
        if self.resume_cnt > 5:
          self.last_resume_frame = frame
          self.resume_cnt = 0
    # reset lead distnce after the car starts moving          
    elif self.last_lead_distance != 0:
      self.last_lead_distance = 0

    elif CS.out.cruiseState.accActive:
      btn_signal = self.NC.update( c, CS, path_plan )
      if btn_signal != None:
        can_sends.append(create_clu11(self.packer, self.resume_cnt, CS.clu11, btn_signal ))
        self.resume_cnt += 1
      else:
        self.resume_cnt = 0
    return  can_sends


  def update(self, c, CS, frame ):
    enabled = c.enabled
    actuators = c.actuators
    left_lane = c.hudControl.leftLaneVisible 
    right_lane = c.hudControl.rightLaneVisible 
    left_lane_warning = c.hudControl.leftLaneDepart 
    right_lane_warning = c.hudControl.rightLaneDepart
    # pcm_cancel_cmd = c.cruiseControl.cancel    
    # vFuture = c.hudControl.vFuture * 3.6
  
    # Steering Torque
    new_steer = int(round(actuators.steer * self.p.STEER_MAX))
    apply_steer = apply_std_steer_torque_limits(new_steer, self.apply_steer_last, CS.out.steeringTorque, self.p)
    self.steer_rate_limited = new_steer != apply_steer

    # disable when temp fault is active, or below LKA minimum speed
    # lkas_active = enabled and not CS.out.steerWarning and CS.out.vEgo >= CS.CP.minSteerSpeed and CS.out.cruiseState.enabled
    path_plan = self.NC.update_lateralPlan()    
    lkas_active = self.lkas_active_control( enabled, CS, path_plan )

    if not lkas_active:
      apply_steer = 0
      self.steer_timer_apply_torque = 0
    else:
      apply_steer = self.smooth_steer(  apply_steer )

    self.apply_steer_last = apply_steer
    sys_warning, sys_state = self.process_hud_alert( lkas_active, c )


    if sys_warning and CS.clu_Vanz < 30:
      sys_warning = False

    if frame == 0: # initialize counts from last received count signals
      self.lkas11_cnt = CS.lkas11["CF_Lkas_MsgCount"] + 1
      self.scc_live =  CS.scc12["CR_VSM_Alive"] 

    self.lkas11_cnt %= 0x10
    self.scc_live %= 0x0F

    can_sends = []
    can_sends.append( create_lkas11(self.packer, self.lkas11_cnt, self.car_fingerprint, apply_steer, lkas_active,
                                   CS.lkas11, sys_warning, sys_state, enabled,
                                   left_lane, right_lane,
                                   left_lane_warning, right_lane_warning) )

    can_sends.append( create_mdps12(self.packer, frame, CS.mdps12) )

    if  CS.CP.openpilotLongitudinalControl:
      can_sends = self.updateLongitudinal( can_sends, c, CS, frame )
    else:
      can_sends = self.update_resume( can_sends, c, CS, frame, path_plan )

      if (frame % 2 == 0) and CS.CP.atompilotLongitudinalControl and CS.cruise_set_mode == 0:
        can_sends = self.update_scc12( can_sends, c, CS, self.scc_live )

        self.scc_live += 1
      
    # 20 Hz LFA MFA message
    if frame % 5 == 0:
      self.update_debug( CS, c )
      if self.car_fingerprint in FEATURES["send_hda_mfa"]:
        can_sends.append( create_hda_mfc(self.packer, CS, c ) )
      elif self.car_fingerprint in FEATURES["send_lfa_mfa"]:
        can_sends.append( create_lfahda_mfc(self.packer, enabled) )
   
    self.lkas11_cnt += 1

    new_actuators = actuators.copy()
    new_actuators.steer = apply_steer / self.p.STEER_MAX
    new_actuators.accel = self.accel

    return new_actuators, can_sends
