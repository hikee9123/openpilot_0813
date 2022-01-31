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


class ControlPanel : public ListWidget 
{
  Q_OBJECT
public:
  explicit ControlPanel(QWidget* parent = nullptr);

signals:
  void closeSettings();

  
protected:
  void hideEvent(QHideEvent *event) override;
  void showEvent(QShowEvent *event) override;
private:
  QButtonGroup *nav_btns;
  QStackedWidget *panel_widget;


private:
  void  reboot();
  void  poweroff();
};

