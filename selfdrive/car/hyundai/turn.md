    1. indi
    
    1.1 lateralTuning.indi.actuatorEffectiveness
      As effectiveness increases, actuation strength decreases 
      Too high: weak, sloppy lane centering, slow oscillation, can't follow high curvature, high steering error causes snappy corrections 
      Too low: overpower, saturation, jerky, fast oscillation 
      Just right: Highest still able to maintain good lane centering. 

    1.2 lateralTuning.indi.timeConstant 
      Extend exponential decay of prior output steer 
      Too high: sloppy lane centering 
      Too low: noisy actuation, responds to every bump, maybe unable to maintain lane center due to rapid actuation 
      Just right: above noisy actuation and lane centering instability 
    
    
    1.3 lateralTuning.indi.innerLoopGain 
      Steer rate error gain 
      Too high: jerky oscillation in high curvature 
      Too low: sloppy, cannot accomplish desired steer angle 
      Just right: brief snap on entering high curvature 

    1.4 lateralTuning.indi.outerLoopGain 
      Steer error gain 
      Too high: twitchy hyper lane centering, oversteering 
      Too low: sloppy, all over lane 
      Just right: crisp lane centering


    1.5  ex
      ret.lateralTuning.init('indi')
      ret.lateralTuning.indi.innerLoopGainBP = [0.]
      ret.lateralTuning.indi.innerLoopGainV = [3.5]
      ret.lateralTuning.indi.outerLoopGainBP = [0.]
      ret.lateralTuning.indi.outerLoopGainV = [2.0]
      ret.lateralTuning.indi.timeConstantBP = [0.]
      ret.lateralTuning.indi.timeConstantV = [1.4]               # 2. 중앙 조정
      ret.lateralTuning.indi.actuatorEffectivenessBP = [0.]
      ret.lateralTuning.indi.actuatorEffectivenessV = [5.3]      # 1. 낮을수록 코너를 잘돔.  (직진 와리 가리 조정)
  
