
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
  // setup two main layouts
  //sidebar_widget = new QWidget;
  //QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar_widget);
  //sidebar_layout->setMargin(0);
 
  QHBoxLayout *main_layout = new QHBoxLayout(this); 

  // setup panels
   QList<QPair<QString, func *>> panels = {
    {"Reboot", &ControlPanel::reboot},
    {"Power Off", &ControlPanel::poweroff},
  };

  const int padding = 35;

  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setStyleSheet(QString(R"(
      QPushButton {
        color: grey;
        font-size: 65px;
        font-weight: 500;
        height: 120px;
         background-color: #393939;
      }
      QPushButton:checked {
        color: white;
      }
      QPushButton:pressed {
        color: #ADADAD;
        background-color: #4a4a4a; 
      }
    )").arg(padding));


    main_layout->addWidget(btn);//, 0, Qt::AlignRight);

    QObject::connect(btn, &QPushButton::clicked, [=, func]() {
      btn->setChecked(true);
     // main_layout->setCurrentWidget(w);
    });
  }


  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    ControlPanel {
      background-color: black;
    }
  )");    
  
/*
  // power buttons
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
*/  
  addItem(main_layout);

}

void ControlPanel::showEvent(QShowEvent *event) {
  panel_widget->setCurrentIndex(0);
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