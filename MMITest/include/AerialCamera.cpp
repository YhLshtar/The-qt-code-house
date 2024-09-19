#include <QThread>
#include <QDebug>
#include <QTime>
#include <QtConcurrent>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "include/AerialCamera.h"

unsigned int Lwidth  = 768;
unsigned int Lheight = 200;
unsigned int Hwidth  = 1024;
unsigned int Hheight = 620;

AerialCamera::AerialCamera()
{
    qDebug()<<"AerialCamera Thread ID :"<<QThread::currentThreadId();

    CameraName  = "/dev/video11";
    camera_ctrl = new CameraControl(7 , 1); //i2c7
    m_timer     = new QTimer();

    connect(m_timer, &QTimer::timeout, this, [=]
    {
        if(!camera_ctrl->WriteCommand(0x6030, 0x1))
        {
            qDebug("I2C Error, SoftWare Trigger Failed");
        }
        else
        {
//            qDebug("ST~");
        }
    });

    imgData = new unsigned char[width * height];
    for(int i = 0; i < 200; i++)
    {
        imgDataVector[i] = new unsigned char[Lwidth * Hheight];
    }

    qDebug("AerialCamera Create, CameraName: %s", CameraName);
}

AerialCamera::~AerialCamera()
{
    CloseCamera();
    for(int i = 0; i < 200; i++)
    {
        delete imgDataVector[i];
    }
    delete imgData;
    delete camera_ctrl;
    qDebug("AerialCamera Delete");
}

bool AerialCamera::OpenCamera()
{
    if(camera_ctrl->OpenDev())
    {
        unsigned int rdata = 0;
        /* 配置相机输出禁止输出测试图 */
        bool wStatus = camera_ctrl->WriteCommand(0x0108, 0x0);
        QThread::msleep(100);
        /* 读取相机当前的测试图格式 */
        bool rStatus = camera_ctrl->ReadCommand(0x0108, rdata, 1000);
        if(wStatus == 0 || rStatus == 0)
        {
            qDebug("I2C Set Camera Error!~~ wStatus:%d rStatus:%d rvalue:%d\n", wStatus, rStatus, rdata);
            return false;
        }
        else
        {
            qDebug("I2C Set Camera Ok, wStatus:%d rStatus:%d rvalue:%d\n", wStatus, rStatus, rdata);
            camera_ctrl->WriteCommand(0x0808, 1);          // 0: 内触发,内部一直触发 1: 外触发,需要外部给出触发信号或软件触发
            camera_ctrl->WriteCommand(0x051C, yCapture);   // 1次触发就会拍摄CAPTURE_NO(1000)帧图片
            camera_ctrl->WriteCommand(0x0000, yGain);      // 设置增益(0.1db-48db)<---->(0-480)
            camera_ctrl->WriteCommand(0x0014, yRoiX);      // 图像 水平 偏移（范围0-812）
            camera_ctrl->WriteCommand(0x001C, yRoiY);      // 图像 垂直 偏移（范围0-620）
            unsigned int frameInterval = 1000000 / yFps;
            /*
             * 0x0048寄存器使能后，修改帧率的同时也会改变mipi传输速率，但实测影响并不是很大，最多相差几十ms
             * 可以尝试修改完帧率后再把0x0048寄存器关闭，改回原来的帧率，但是又需要测试帧率是不是也被修改了
            */
            if (!camera_ctrl->WriteCommand(0x0048, 1) || !camera_ctrl->WriteCommand(0x004C, frameInterval))
            {
                qDebug("Failed To Set FrameRate!~~");
            }
            CameraOpen = true;
        }
    }
    else
    {
        qDebug("Camera Control Error, I2C Not Found!~~");
        return false;
    }

    v_capt = new VideoCapture(CameraName);

    return true;
}

void AerialCamera::CloseCamera()
{
    PreviewTask_stop();
    if(v_capt != nullptr)
    {
//        if(!camera_ctrl->WriteCommand(0x6030, 0x1))
//        {
//            qDebug("I2C Error, SoftWare Trigger Failed");
//        }
//        unsigned char* p_img = v_capt->getImage();
        v_capt->stopCapture();
        delete v_capt;
        v_capt = nullptr;
        qDebug("AerialCamera Stop");
    }
    camera_ctrl->CloseDev();
    CameraOpen = false;
}

bool AerialCamera::isCameraOpen()
{
    return CameraOpen;
}

void AerialCamera::PreviewTask_start()
{
    is_task_start = true;
}

void AerialCamera::PreviewTask_stop()
{
    is_task_start = false;
}

void AerialCamera::quit_task()
{
    StopPreview();
    is_task_start = false;
    is_task_quit  = true;
    if(!camera_ctrl->WriteCommand(0x6030, 0x1))
    {
        qDebug("I2C Error, SoftWare Trigger Failed");
    }
}

void AerialCamera::run()
{
    qDebug()<<"AerialCamera Preview Thread ID :"<<QThread::currentThreadId();
    while(1)
    {
        if(is_task_start && CameraOpen)
        {
//            qDebug("1");
            unsigned char* p_img = v_capt->getImage();
//            qDebug("2");
            if(is_task_start)
            {
                if(p_img)
                {
                    imgDataMutex.lock();
                    memcpy(imgData, p_img, width * height);
                    if(v_capt != nullptr)
                    {
                        v_capt->putImage(p_img);
                    }
                    imgDataMutex.unlock();
                }
            }
        }
        else
        {
            QThread::msleep(1500);
            qDebug("AerialCamera  Preview Thread Suspend...");
            if(is_task_quit)
            {
                break;
            }
        }
    }
    qDebug("Quit Preview Task");
}

bool AerialCamera::SetCapture(int cap)
{
    if(!camera_ctrl->WriteCommand(0x051C, cap))
    {
        return false;
    }
    return true;
}

bool AerialCamera::ReadCapture(unsigned int &cap)
{
    if(!camera_ctrl->ReadCommand(0x051C, cap, 1000))
    {
        return false;
    }
    return true;
}

bool AerialCamera::SetGain(int gain)
{
    if(!camera_ctrl->WriteCommand(0x0000, gain))
    {
        return false;
    }
    return true;
}

bool AerialCamera::ReadGain(unsigned int &gain)
{
    if(!camera_ctrl->ReadCommand(0x0000, gain, 1000))
    {
        return false;
    }
    return true;
}

bool AerialCamera::SetFPS(int fps)
{
    unsigned int frameInterval = 1000000 / fps;
    if (!camera_ctrl->WriteCommand(0x0048, 1) || !camera_ctrl->WriteCommand(0x004C, frameInterval))
    {
        return false;
    }
    return true;
}

bool AerialCamera::ReadFPS(unsigned int &fps)
{
    if(!camera_ctrl->ReadCommand(0x0000, fps, 1000))
    {
        return false;
    }
    return true;
}

bool AerialCamera::SetExposure(int exposure)
{
    if(!camera_ctrl->WriteCommand(0x0044, exposure))
    {
        return false;
    }
    return true;
}

bool AerialCamera::ReadExposure(unsigned int &exposure)
{
    if(!camera_ctrl->ReadCommand(0x0044, exposure, 1000))
    {
        return false;
    }
    return true;
}

bool AerialCamera::SetROI(int x, int y)
{
    if(!camera_ctrl->WriteCommand(0x0014, x) || !camera_ctrl->WriteCommand(0x001C, y))
    {
        return false;
    }
    return true;
}

bool AerialCamera::ReadROI(unsigned int &x, unsigned int &y)
{
    if(!camera_ctrl->ReadCommand(0x0014, x, 1000) || !camera_ctrl->ReadCommand(0x001C, y, 1000))
    {
        return false;
    }
    return true;
}

bool AerialCamera::SetResolution(char mode)
{
    QString m_str;

    if(v_capt != nullptr)
    {
        if(!camera_ctrl->WriteCommand(0x6030, 0x1))
        {
            qDebug("I2C Error, SoftWare Trigger Failed");
        }
        unsigned char* p_img = v_capt->getImage();
        QCoreApplication::processEvents();
        msleep(500);
        v_capt->stopCapture();
        qDebug("AerialCamera Stop");
    }

    switch(mode)
    {
        case 0:
        {
            width  = Hwidth;
            height = Hheight;
            isCutMode = 0;
            if(!camera_ctrl->WriteCommand(0x0010, isCutMode))   //0: 全分辨率模式即812x620
            {
                qDebug("全分辨率模式, Set Error!~~");
                return false;
            }
            system(R"_(media-ctl -d /dev/media1 --set-v4l2 '"m00_b_sc530ai 1-0030":0[fmt:SBGGR8_1X8/812x620]')_");
            system(R"_(media-ctl -d /dev/media1 --set-v4l2 '"rockchip-csi2-dphy0":0[fmt:SBGGR8_1X8/812x620]')_");
            system(R"_(media-ctl -d /dev/media1 --set-v4l2 '"rockchip-mipi-csi2":0[fmt:SBGGR8_1X8/812x620]')_");
            m_str = "v4l2-ctl -d " + QString(CameraName) +
                    " --set-selection=target=crop,top=0,left=0,width=812,height=620";
        }break;
        case 1:
        {
            yRoiW  = Lwidth;
            yRoiH  = Lheight;
            width  = Lwidth;
            height = Lheight;
            isCutMode = 1;
            if(!camera_ctrl->WriteCommand(0x0010, isCutMode) || // 1: 裁剪模式默认600x200
               !camera_ctrl->WriteCommand(0x0018, yRoiW)     || // 图像 水平 宽度（范围0-812）, 全分辨率模式水平宽度不生效
               !camera_ctrl->WriteCommand(0x0020, yRoiH))       // 图像 垂直 宽度（范围0-620）, 全分辨率模式垂直宽度不生效
            {
               qDebug("裁剪模式, Set Error!~~");
               return false;
            }
            system(R"_(media-ctl -d /dev/media1 --set-v4l2 '"m00_b_sc530ai 1-0030":0[fmt:SBGGR8_1X8/768x200]')_");
            system(R"_(media-ctl -d /dev/media1 --set-v4l2 '"rockchip-csi2-dphy0":0[fmt:SBGGR8_1X8/768x200]')_");
            system(R"_(media-ctl -d /dev/media1 --set-v4l2 '"rockchip-mipi-csi2":0[fmt:SBGGR8_1X8/768x200]')_");
            m_str = "v4l2-ctl -d " + QString(CameraName) +
                    " --set-selection=target=crop,top=0,left=0,width=768,height=200";
        }break;
    }

    qDebug() << m_str;
    system(m_str.toLatin1());

    delete imgData;
    imgData = new unsigned char[width * height];

    if(v_capt != nullptr)
    {
        v_capt->startCapture();
    }

    return true;
}

QImage AerialCamera::getImage()
{
//    imgDataMutex.lock();
    return QImage(imgData, width, height,
                  cv::Mat(height, width, CV_8UC1, imgData).step,
                  QImage::Format_Grayscale8);
//    imgDataMutex.unlock();
}

signed char AerialCamera::Capture(int cap)
{
    if(!CameraOpen)
    {
        return 0;
    }
    capture_cnt = cap;
    do
    {
        unsigned char* p_img = v_capt->getImage();
        if(p_img)
        {
            memcpy(imgDataVector[cap - capture_cnt], p_img, v_capt->frame_size);
            v_capt->putImage(p_img);
            capture_cnt--;
            qDebug("dequeue length = %u, index = %u, buff = %u", v_capt->dequeue_length, v_capt->dequeue_index, capture_cnt);
        }
        else
        {
            //usleep(10 * 1000);
        }
    }while(capture_cnt > 1);

    return 1;
}

void AerialCamera::SaveImage()
{
    if(isCutMode)
    {
        cv::imwrite("768X200.jpg", cv::Mat(Lheight, Lwidth, CV_8UC1, imgData));
    }
    else
    {
        cv::imwrite("1024X620.jpg", cv::Mat(Hheight, Hwidth, CV_8UC1, imgData));
    }
}

void AerialCamera::StartPreview()
{
    PreviewTask_start();
    m_timer->start(25);
}

void AerialCamera::StopPreview()
{
    PreviewTask_stop();
    m_timer->stop();
}

void AerialCamera::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}
