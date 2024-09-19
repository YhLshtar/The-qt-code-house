#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QTime>
#include <QDebug>
#include <QCoreApplication>
#include "AxleCamera.h"

AxleCamera::AxleCamera()
{
    qDebug()<<"AxleCamera Thread ID :"<<QThread::currentThreadId();

    CameraName = "/dev/video22";

    v_capt  = new AxleVideoCapture(CameraName);
    imgData = new unsigned char[1];

    qDebug("AxleCamera Create, CameraName: %s", CameraName);
}

AxleCamera::~AxleCamera()
{
    CloseCamera();
    delete imgData;
    delete v_capt;
    qDebug("AxleCamera Delete");
}

bool AxleCamera::OpenCamera()
{
    if(SetResolution(640, 480))
    {
        qDebug("AxleCamera Open");
    }
    else
    {
        return false;
    }

    return true;
}

void AxleCamera::CloseCamera()
{
    if(v_capt->stopCapture())
    {
        CameraOpen = false;
        qDebug("AxleCamera Close");
    }
    else
    {

    }
}

bool AxleCamera::isCameraOpen()
{
    return CameraOpen;
}

void AxleCamera::PreviewTask_start()
{
    is_task_start = true;
}

void AxleCamera::PreviewTask_stop()
{
    is_task_start = false;
}

void AxleCamera::quit_task()
{
    StopPreview();
    is_task_start = false;
    is_task_quit  = true;
}

void AxleCamera::run()
{
    qDebug()<<"AxleCamera Preview Thread ID :"<<QThread::currentThreadId();
    while(1)
    {
        if(is_task_start && v_capt->isCameraOpen())
        {
            unsigned char* p_img = v_capt->getImage();
            if(is_task_start)
            {
                if(p_img)
                {
                    imgDataMutex.lock();
                    memcpy(imgData, p_img, v_capt->height * v_capt->width * 2);
//                    memcpy(imgData, p_img, v_capt->frame_size);
                    imgDataMutex.unlock();
                }
            }
        }
        else
        {
            QThread::msleep(1500);
            qDebug("AxleCamera Preview Thread Suspend...");
            if(is_task_quit)
            {
                break;
            }
        }
    }
    qDebug("Quit Preview Task");
}

bool AxleCamera::SetResolution(int w, int h)
{
    QString m_str;

    if(v_capt->isCameraOpen())
    {
        v_capt->stopCapture();
        qDebug("AxleCamera Stop");
    }
    else
    {

    }

    m_str = "v4l2-ctl -d /dev/video22 --set-fmt-video=width=" + QString::number(w)+ ",height=" + QString::number(h) + ",pixelformat=UYVY";
    qDebug()<<m_str;
    system(m_str.toLatin1());

    signed char ret = v_capt->startCapture();

    if( ret > 0)
    {
        delete imgData;
        imgData = new unsigned char[v_capt->frame_size];
    }
    else
    {
        qDebug("ret = %d", ret);
        return false;
    }

    return true;
}

QImage AxleCamera::getImage()
{
    return QImage(imgData, v_capt->width, v_capt->height,
                  cv::Mat(v_capt->height, v_capt->width, CV_8UC2, imgData).step,
                  QImage::Format_Grayscale16);
}

void AxleCamera::SaveImage()
{
    cv::imwrite("648X480.jpg", cv::Mat(v_capt->height, v_capt->width, CV_8UC2, imgData));
}

void AxleCamera::StartPreview()
{
    PreviewTask_start();
}

void AxleCamera::StopPreview()
{
    PreviewTask_stop();
}

void AxleCamera::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}
