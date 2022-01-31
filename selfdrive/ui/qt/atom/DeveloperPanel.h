#pragma once



#include <QWidget>
#include <QFrame>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QScrollArea>
#include <QStackedWidget>

#include "selfdrive/ui/qt/widgets/controls.h"


class DeveloperPanel : public QFrame 
{
  Q_OBJECT
public:
  explicit DeveloperPanel(QWidget* parent = nullptr);


protected:
  void showEvent(QShowEvent *event) override;

};



////////////////////////////////////////////////////////////////////////

class CLiveSteerRatioToggle : public AbstractControl {
  Q_OBJECT

public:
  CLiveSteerRatioToggle();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};

class CPrebuiltToggle : public ToggleControl {
  Q_OBJECT

public:
  CPrebuiltToggle() : ToggleControl("Prebuilt 파일 생성", "Prebuilt 파일을 생성하며 부팅속도를 단축시킵니다. UI수정을 한 경우 기능을 끄십시오.", "../assets/offroad/icon_shell.png", Params().getBool("PutPrebuiltOn")) {
    QObject::connect(this, &CPrebuiltToggle::toggleFlipped, [=](int state) {
      Params().putBool("PutPrebuiltOn", (bool)state);

      if( state )
      {
        std::system( "cd /data/openpilot; touch prebuilt" );
      }
      else
      {
        std::system( "cd /data/openpilot; rm -f prebuilt" );
      }
 
    });
  }
};



class BrightnessControl : public AbstractControl 
{
  Q_OBJECT

public:
  BrightnessControl();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};

class CVolumeControl : public AbstractControl {
  Q_OBJECT

public:
  CVolumeControl();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};



class BrightnessOffControl : public AbstractControl {
  Q_OBJECT

public:
  BrightnessOffControl();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;
  Params params;
  
  void refresh();
};


class AutoScreenOff : public AbstractControl {
  Q_OBJECT

public:
  AutoScreenOff();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};

class CAutoFocus : public AbstractControl {
  Q_OBJECT

public:
  CAutoFocus();

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};
////////////////////////////////////////////////////////////////////////////////////////
//
//  Git

class GitHash : public AbstractControl {
  Q_OBJECT

public:
  GitHash();

private:
  QLabel local_hash;
  QLabel remote_hash;
};



class SshLegacyToggle : public ToggleControl {
  Q_OBJECT

public:
  SshLegacyToggle() : ToggleControl("기존 공개KEY 사용", "SSH 접속시 기존 공개KEY(0.8.2이하)를 사용합니다.", "", Params().getBool("OpkrSSHLegacy")) {
    QObject::connect(this, &SshLegacyToggle::toggleFlipped, [=](int state) {
      char value = state ? '1' : '0';
      Params().put("OpkrSSHLegacy", &value, 1);

      if( state )
      {
        std::system("cp -f /data/openpilot/selfdrive/assets/addon/key/GithubSshKeys_legacy /data/params/d/GithubSshKeys; chmod 600 /data/params/d/GithubSshKeys; touch /data/public_key");
      }
      else
      {
        std::system("cp -f /data/openpilot/selfdrive/assets/addon/key/GithubSshKeys /data/params/d/GithubSshKeys; chmod 600 /data/params/d/GithubSshKeys; rm -f /data/public_key");
      }

    });
  }
};

class IsOpenpilotViewEnabledToggle : public ToggleControl {
  Q_OBJECT

public:
   IsOpenpilotViewEnabledToggle();

};
////////////////////////////////////////////////////////////////////////////////////////
//
//  Combo box
class CarSelectCombo : public AbstractControl 
{
  Q_OBJECT

public:
  CarSelectCombo();

public:
  QComboBox  combobox;

  void refresh();

};

class CarSelectBtn : public AbstractControl 
{
  Q_OBJECT

public:
  CarSelectBtn( CarSelectCombo *pCarSelectmenu );


  CarSelectCombo *m_pCarSelectMenu;

private:
  QPushButton btnplus;
  QPushButton btnminus;
  QLabel label;

  void refresh();
};

