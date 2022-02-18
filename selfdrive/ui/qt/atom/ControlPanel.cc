
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <QHBoxLayout>


#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/toggle.h"
#include "selfdrive/ui/qt/widgets/offroad_alerts.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"

#include "selfdrive/common/params.h"
#include "selfdrive/common/util.h"
#include "selfdrive/ui/ui.h"

#include "ControlPanel.h"

// 일부 코드 OPKR 참고.
#include "atom/DeveloperPanel.h"
#include "atom/CommunityPanel.h"


ControlPanel::ControlPanel(QWidget* parent) : ListWidget(parent)
{
  // power buttons
  QHBoxLayout *main_layout = new QHBoxLayout(this);
  main_layout->setSpacing(30);

  QPushButton *reboot_btn = new QPushButton("Reboot");
  reboot_btn->setObjectName("reboot_btn");
  main_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::clicked, this, &ControlPanel::reboot);

  QPushButton *poweroff_btn = new QPushButton("Power Off");
  poweroff_btn->setObjectName("poweroff_btn");
  main_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::clicked, this, &ControlPanel::poweroff);

  setStyleSheet(R"(
    #reboot_btn { height: 120px; border-radius: 15px; background-color: #393939; }
    #reboot_btn:pressed { background-color: #4a4a4a; }
    #poweroff_btn { height: 120px; border-radius: 15px; background-color: #E22C2C; }
    #poweroff_btn:pressed { background-color: #FF2424; }
  )");

  addItem(main_layout);

}

void ControlPanel::showEvent(QShowEvent *event) {

}


void ControlPanel::hideEvent(QHideEvent *event) {
#ifdef QCOM
  //HardwareEon::close_activities();
#endif
}


void ControlPanel::reboot() {
  if (uiState()->status == UIStatus::STATUS_DISENGAGED) {
    if (ConfirmationDialog::confirm("Are you sure you want to reboot?", this)) {
      // Check engaged again in case it changed while the dialog was open
      if (uiState()->status == UIStatus::STATUS_DISENGAGED) {
        Params().putBool("DoReboot", true);
      }
    }
  } else {
    ConfirmationDialog::alert("Disengage to Reboot", this);
  }
}

void ControlPanel::poweroff() {
  if (uiState()->status == UIStatus::STATUS_DISENGAGED) {
    if (ConfirmationDialog::confirm("Are you sure you want to power off?", this)) {
      // Check engaged again in case it changed while the dialog was open
      if (uiState()->status == UIStatus::STATUS_DISENGAGED) {
        Params().putBool("DoShutdown", true);
      }
    }
  } else {
    ConfirmationDialog::alert("Disengage to Power Off", this);
  }
}