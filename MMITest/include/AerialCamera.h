#ifndef AERIALCAMERA_H
#define AERIALCAMERA_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QImage>
#include <QMutex>
#include "libraries/CameraControl.h"
#include "libraries/VideoCapture.h"

class AerialCamera : public QThread
{
    Q_OBJECT

public:
    explicit AerialCamera();
    ~AerialCamera();

    bool OpenCamera();
    void CloseCamera();
    bool isCameraOpen();

    bool SetResolution(char mode = 0);
    bool SetCapture(int cap);
    bool ReadCapture(unsigned int &cap);
    bool SetGain(int gain);
    bool ReadGain(unsigned int &gain);
    bool SetFPS(int fps);
    bool ReadFPS(unsigned int &fps);
    bool SetExposure(int exposure);
    bool ReadExposure(unsigned int &exposure);
    bool SetROI(int x, int y);
    bool ReadROI(unsigned int &x, unsigned int &y);
    void SaveImage();
    signed char Capture(int cap);
    void StopPreview();
    void StartPreview();
    void quit_task();

    QImage getImage();

public:
    unsigned char* imgDataVector[200];

protected:
    void msleep(int msec);
    void PreviewTask_start();
    void PreviewTask_stop();

    void run() override;

protected:
    unsigned int width     = 1024;
    unsigned int height    = 620;

    unsigned int isCutMode = 0;
    unsigned int yCapture  = 2;
    unsigned int yRoiW     = 0;
    unsigned int yRoiH     = 0;

    unsigned int yRoiX     = 22;
    unsigned int yRoiY     = 320;
    unsigned int yFps      = 100;
    unsigned int yGain     = 0;

private:
    CameraControl*   camera_ctrl = nullptr;
    ::VideoCapture*  v_capt      = nullptr;
    QTimer*          m_timer     = nullptr;
    const char*      CameraName  = nullptr;
    unsigned char*   imgData     = nullptr;
    signed int       capture_cnt = 0;

    bool is_task_start   = false;
    bool is_task_quit    = false;
    bool CameraOpen      = false;

    QMutex imgDataMutex;

};

#endif // AERIALCAMERA_H
