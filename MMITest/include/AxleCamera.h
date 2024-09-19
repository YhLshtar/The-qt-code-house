#ifndef AXLECAMERA_H
#define AXLECAMERA_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QMutex>
#include "libraries/AxleVideoCapture.h"

class AxleCamera : public QThread
{
    Q_OBJECT

public:
    explicit AxleCamera();
    ~AxleCamera();

    bool OpenCamera();
    void CloseCamera();
    bool isCameraOpen();

    bool SetResolution(int w, int h);
    void SaveImage();
    void StopPreview();
    void StartPreview();
    void quit_task();

    QImage getImage();

protected:
    void msleep(int msec);
    void PreviewTask_start();
    void PreviewTask_stop();

    void run() override;

private:
    AxleVideoCapture* v_capt      = nullptr;
    unsigned char*    imgData     = nullptr;
    const char*       CameraName  = nullptr;

    bool is_task_start   = false;
    bool is_task_quit    = false;
    bool CameraOpen      = false;

    QMutex imgDataMutex;

};

#endif // AXLECAMERA_H
