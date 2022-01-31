
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
// #include <QPushButton>


#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/toggle.h"
#include "selfdrive/ui/qt/widgets/offroad_alerts.h"
#include "selfdrive/ui/qt/widgets/controls.h"

#include "selfdrive/common/params.h"
#include "selfdrive/common/util.h"
#include "selfdrive/ui/ui.h"

#include "CommunityPanel.h"

// 일부 코드 OPKR 참고.



CommunityPanel::CommunityPanel(QWidget* parent) : ListWidget(parent)
{
  // param, title, desc, icon
  std::vector<std::tuple<QString, QString, QString, QString>> toggles{
    {
      "UploadRaw",
      "Upload Raw Logs",
      "Upload full logs and full resolution video by default while on Wi-Fi. If not enabled, individual logs can be marked for upload at useradmin.comma.ai.",
      "../assets/offroad/icon_network.png",
    },
      
    {
      "OpkratomLongitudinal",
      "Enable atom Longitudinal",
      "Use the openpilot longitudinal system for adaptive cruise control and lane keep driver assistance.  Changing this setting takes effect when the car is powered off.",
      "../assets/offroad/icon_speed_limit.png",
    },

    {
      "OpkrAutoResume",
      "자동출발 기능 사용",
      "SCC 사용중 정차시 자동출발 기능을 사용합니다.",
      "../assets/offroad/icon_shell.png",
    },

    {
      "OpkrRunNaviOnBoot",
      "부팅 후 네비 자동 실행",
      "부팅후 네비게이션(티맵)을 자동 실행합니다.",
      "../assets/offroad/icon_shell.png",
    },    

    {
      "OpkrTurnSteeringDisable",
      "턴시그널 사용시 조향해제 사용",
      "차선변경속도 이하로 주행할 때 턴시그널을 사용시 자동조향을 일시해제 합니다.",
      "../assets/offroad/icon_shell.png",
    },    

    {
      "OpkrPandaFirmwareCk",
      "Panda Firmware Check",
      "판다의 FirmWare를 확인합니다.",
      "../assets/offroad/icon_shell.png",
    },

    {
      "OpkrPowerShutdown",
      "Power off and shutdown",
      "전원 미공급후 배터리 잔량 70퍼센트 이하일경우 전원을 off합니다..",
      "../assets/offroad/icon_shell.png",
    },  

  };

  for (auto &[param, title, desc, icon] : toggles) {
    auto toggle = new ParamControl(param, title, desc, icon, this);
    bool locked = Params().getBool((param + "Lock").toStdString());
    toggle->setEnabled(!locked);
    if (!locked) {
     // connect(uiState(), &UIState::offroadTransition, toggle, &ParamControl::setEnabled);
    }
    addItem(toggle);
    //layout()->addWidget(toggle);
  }

}
