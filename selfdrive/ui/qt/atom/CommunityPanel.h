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


class CommunityPanel : public QFrame 
{
  Q_OBJECT
public:
  explicit CommunityPanel(QWidget* parent = nullptr);


protected:
  void showEvent(QShowEvent *event) override;

};

