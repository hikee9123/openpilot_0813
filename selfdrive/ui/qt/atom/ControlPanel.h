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

private:
  QButtonGroup *nav_btns;

private:
  void  reboot();
  void  poweroff();
};

