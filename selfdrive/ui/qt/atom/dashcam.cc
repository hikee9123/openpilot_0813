#include <time.h>
#include <dirent.h>

#include "selfdrive/ui/qt/atom/dashcam.h"

#include <cmath>

#include <QDateTime>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QVBoxLayout>


#include "selfdrive/common/timing.h"
#include "selfdrive/ui/qt/util.h"



const Rect btn_dashcam_rec = {1745, 860, 140, 140};


OnDashCam::OnDashCam(QWidget *parent) : QWidget(parent) 
{


}

void OnDashCam::updateState(const UIState &s)
{
  double cur_draw_t = millis_since_boot();
  double dt = cur_draw_t - prev_draw_t;
  if (dt < 200)  return;
  prev_draw_t = cur_draw_t;

 


  update_dashcam( s );

 // update(); 

}



void OnDashCam::paintEvent(QPaintEvent *event) 
{
  QPainter p(this);

  screen_draw_button( p );
}


void OnDashCam::mousePressEvent(QMouseEvent* e) 
{
  int e_x = e->x();
  int e_y = e->y();
  //int e_button= e->button();

   printf("OnDashCam::mousePressEvent %d,%d  \n", e_x, e_y);

  if( btn_dashcam_rec.ptInRect( e_x, e_y ) ) 
  {
    printf( "  captureState = %d \n", captureState );
    screen_toggle_record_state();
  }
}

void OnDashCam::drawText(QPainter &p, int x, int y, const QString &text, QColor qColor, int nAlign ) 
{
  QFontMetrics fm(p.font());
  QRect init_rect = fm.boundingRect(text);
  QRect real_rect = fm.boundingRect(init_rect, 0, text);

  real_rect.moveCenter({x, y});
  p.setPen( qColor ); 
  //p.drawText(real_rect.x(), real_rect.bottom(), text);
  p.drawText(real_rect, nAlign, text);
}


struct tm OnDashCam::get_time_struct()
{
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  return tm;
}



int OnDashCam::get_time()
{
  int iRet;
  struct timeval tv;
  int seconds = 0;

  iRet = gettimeofday(&tv, NULL);
  if (iRet == 0)
  {
    seconds = (int)tv.tv_sec;
  }
  return seconds;
}


void OnDashCam::remove_file(char *videos_dir, char *filename)
{
  if (filename[0] == '\0')
  {
    // Don't do anything if no filename is passed
    return;
  }

  int status;
  char fullpath[64];
  snprintf(fullpath, sizeof(fullpath), "%s/%s", videos_dir, filename);
  status = remove(fullpath);
  if (status == 0)
  {
    printf("Removed file: %s\n", fullpath);
  }
  else
  {
    printf("Unable to remove file: %s\n", fullpath);
    perror("Error message:");
  }
}

void OnDashCam::save_file(char *videos_dir, char *filename)
{
  if (!strlen(filename))
  {
    return;
  }

  // Rename file to save it from being overwritten
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "mv %s/%s %s/saved_%s", videos_dir, filename, videos_dir, filename);
  printf("save: %s\n", cmd);
  system(cmd);
}

void OnDashCam::stop_capture() 
{
  char videos_dir[50] = "/storage/emulated/0/videos";

  

  if (captureState == CAPTURE_STATE_CAPTURING)
  {
    printf("stop_capture()\n ");
    system("killall -SIGINT screenrecord");
    captureState = CAPTURE_STATE_NOT_CAPTURING;
    elapsed_time = nCurrTimeSec - start_time;
    if (elapsed_time < 3)
    {
      remove_file(videos_dir, filenames[captureNum]);
    }
    else
    {
      //printf("Stop capturing screen\n");
      captureNum++;

      if (captureNum > RECORD_FILES - 1)
      {
        captureNum = 0;
      }
    }
  }
}

void OnDashCam::start_capture()
{
  captureState = CAPTURE_STATE_CAPTURING;
  char cmd[128] = "";
  char videos_dir[50] = "/storage/emulated/0/videos";

  printf("start_capture()\n ");

  //////////////////////////////////
  // NOTE: make sure videos_dir folder exists on the device!
  //////////////////////////////////
  struct stat st = {0};
  if (stat(videos_dir, &st) == -1)
  {
    mkdir(videos_dir, 0700);
  }

  if (strlen(filenames[captureNum]) && files_created >= RECORD_FILES)
  {
    if (locked_files[captureNum] > 0)
    {
      save_file(videos_dir, filenames[captureNum]);
    }
    else
    {
      // remove the old file
      remove_file(videos_dir, filenames[captureNum]);
    }
    locked_files[captureNum] = 0;
  }

  char filename[64];
  struct tm tm = get_time_struct();
  snprintf(filename, sizeof(filename), "%04d%02d%02d-%02d%02d%02d.mp4", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  
  if(Hardware::TICI()) 
  {
     //snprintf(cmd,sizeof(cmd),"screenrecord --size 2160x1080 --bit-rate 7000000 %s/%s&",videos_dir,filename); 
     snprintf(cmd,sizeof(cmd),"screenrecord --size 1440x720 --bit-rate 5000000 %s/%s&",videos_dir,filename); 
  }
  else 
  {
   // snprintf(cmd,sizeof(cmd),"screenrecord --size 1280x720 --bit-rate 4000000 %s/%s&",videos_dir,filename);
    snprintf(cmd, sizeof(cmd), "screenrecord --size 1280x720 --bit-rate 5000000 %s/%s&", videos_dir, filename);
    //snprintf(cmd,sizeof(cmd),"screenrecord --size 960x540 --bit-rate 5000000 %s/%s&",videos_dir,filename);
  }
  strcpy(filenames[captureNum], filename);

  printf("Capturing to file: %s\n", cmd);
  start_time = nCurrTimeSec;
  system(cmd);

  if (lock_current_video)
  {
    // Lock is still on so mark this file for saving
    locked_files[captureNum] = 1;
  }
  else
  {
    locked_files[captureNum] = 0;
  }

  files_created++;
}





/*
bool OnDashCam::screen_button_clicked(  Rect rect )
{
  UIState *s = uiState();

  static int touch_cnt_old = -1 ;

  int x = s->scene.mouse.touch_x;
  int y = s->scene.mouse.touch_y;
  int touch_cnt = s->scene.mouse.touch_cnt;

  if( touch_cnt_old == -1 )
  {
    touch_cnt_old = touch_cnt;
    return false;
  }

  if( touch_cnt == touch_cnt_old ) return false;
  touch_cnt_old = touch_cnt;

  if( rect.ptInRect( x, y ) )
  {
    return true;
  }

  return false;
}
*/

void OnDashCam::rotate_video()
{
  // Overwrite the existing video (if needed)
  elapsed_time = 0;
  stop_capture();
  captureState = CAPTURE_STATE_CAPTURING;
  start_capture();
}


void OnDashCam::screen_toggle_record_state()
{
  //if (captureState == CAPTURE_STATE_CAPTURING)
  if( lock_current_video  )
  {
    stop_capture();
    lock_current_video = false;
  }
  else
  {
    // start_capture();
    lock_current_video = true;
  }
}

void OnDashCam::draw_button( QPainter &p, const QString &string, Rect rect, QColor fillColor, QColor txtColor ) 
{
    int btn_x = rect.x;
    int btn_y = rect.y;
    int btn_w = rect.w;
    int btn_h = rect.h;


    QRect rc( btn_x, btn_y, btn_w, btn_h);
    p.setPen(QPen(QColor(0xff, 0xff, 0xff, 100), 3)); 
    p.setBrush(fillColor);
    p.drawEllipse(btn_x, btn_y, btn_w, btn_h);
    p.setPen(Qt::NoPen);


    int btn_xc = rect.centerX();
    int btn_yc = rect.centerY();

   configFont( p, "Open Sans",  50, "SemiBold");
  // drawText( p, btn_xc, btn_yc, string, txtColor, Qt::AlignCenter | Qt::AlignVCenter  );

  QFontMetrics fm(p.font());
  QRect init_rect = fm.boundingRect(string);
  QRect real_rect = fm.boundingRect(init_rect, 0, string);
  real_rect.moveCenter({btn_xc, btn_yc});
  p.setPen( txtColor );
  p.drawText(real_rect, Qt::AlignCenter | Qt::AlignVCenter, string);   

}

void OnDashCam::screen_draw_button(QPainter &p)
{
   UIState *s = uiState(); 
  const int bb_dmr_w = 180;
  const int bb_dmr_x = 0 + s->fb_w - bb_dmr_w - bdr_s/2;

   //btn_dashcam_rec
  Rect btn_rec = btn_dashcam_rec;
  QColor fillColor = QColor(255, 150, 150, 200);
  QColor txtColor = QColor(255, 255, 255, 100);

  btn_rec.x = bb_dmr_x;

    if ( lock_current_video == false )
    {
       fillColor = QColor( 50, 50, 100, 200);
    }
    else if (captureState == CAPTURE_STATE_CAPTURING)
    {
      fillColor = QColor(255, 0, 0, 150);
    }
    else
    {
      fillColor = QColor(255, 150, 150, 200);
    }
 
   draw_button( p, "REC", btn_rec, fillColor, txtColor ); 
  //if (  screen_button_clicked( btn_rec) )
  //{
  //  printf( "  captureState = %d \n", captureState );
  //  screen_toggle_record_state();
  //}  


  if (captureState == CAPTURE_STATE_CAPTURING)
  {

    elapsed_time = nCurrTimeSec - start_time;
    if (elapsed_time >= RECORD_INTERVAL)
    {
      capture_cnt++;
      if( capture_cnt > 10 )
      {
        stop_capture();
        lock_current_video = false;
      }
      else
      {
        rotate_video(); 
      }
    }    
  }
}





void OnDashCam::update_dashcam(const UIState &s)
{
  nCurrTimeSec =  get_time();
  if (!s.awake) return;
  if (!s.scene.started) return;


  if( lock_current_video  )
  {
    float v_ego = s.scene.car_state.getVEgo();
    int engaged = s.scene.controls_state.getEngageable();

    if(  (lock_current_video == 1) && (v_ego < 0.1 || !engaged) )
    {
      elapsed_time = nCurrTimeSec - stop_time;
      if( captureState == CAPTURE_STATE_CAPTURING && elapsed_time > 2 )
      {
        capture_cnt = 0;
        stop_capture();
      }
    }    
    else if( captureState != CAPTURE_STATE_CAPTURING )
    {
      capture_cnt = 0;
      start_capture();
    }
    else
    {
      stop_time = nCurrTimeSec;
    }
    
  }
  else  if( captureState == CAPTURE_STATE_CAPTURING )
  {
    capture_cnt = 0;
    stop_capture();
  }
}





