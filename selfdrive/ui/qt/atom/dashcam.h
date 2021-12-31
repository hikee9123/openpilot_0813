

#pragma once

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QStackedLayout>
#include <QTimer>
#include <QWidget>
#include <QMouseEvent>

#include "selfdrive/ui/ui.h"




#define CAPTURE_STATE_NONE 0
#define CAPTURE_STATE_CAPTURING 1
#define CAPTURE_STATE_NOT_CAPTURING 2
#define CAPTURE_STATE_PAUSED 3
#define CLICK_TIME 0.2
#define RECORD_INTERVAL 180 // Time in seconds to rotate recordings; Max for screenrecord is 3 minutes
#define RECORD_FILES 40     // Number of files to create before looping over


class OnDashCam : public QWidget 
{
  Q_OBJECT


public:
  explicit OnDashCam(QWidget *parent);
  void updateState(const UIState &s);

private:
  void    mousePressEvent(QMouseEvent* e) override;
  void    paintEvent(QPaintEvent *event) override;

private:
  long nCurrTimeSec = 0;
  int captureState = CAPTURE_STATE_NOT_CAPTURING;
  int captureNum = 0;
  int start_time = 0;
  int stop_time = 0;
  int elapsed_time = 0; // Time of current recording
  char filenames[RECORD_FILES][50]; // Track the filenames so they can be deleted when rotating

  bool lock_current_video = false; // If true save the current video before rotating
  bool locked_files[RECORD_FILES]; // Track which files are locked
  int lock_image;                  // Stores reference to the PNG
  int files_created = 0;
  int  capture_cnt = 0;
  int  program_start = 1;
  double prev_draw_t = 0;




private:
  void    drawText(QPainter &p, int x, int y, const QString &text, QColor qColor = QColor(255,255,255,255), int nAlign = Qt::AlignCenter );

private:
   void   draw_button( QPainter &p, const QString &tring, Rect rect, QColor fillColor, QColor txtColor ) ;
   void   screen_draw_button(QPainter &p);
  // bool   screen_button_clicked(  Rect rect );

private:
   struct tm  get_time_struct();
   int    get_time();
   void   remove_file(char *videos_dir, char *filename);
   void   save_file(char *videos_dir, char *filename);
   void   stop_capture() ;
   void   start_capture();

   void   rotate_video();
   void   screen_toggle_record_state();
   void   update_dashcam(const UIState &s);

private:


};

