#include <QTime>
#include <QDebug>
#include "camerapreview2.h"
#include "ui_camerapreview2.h"

CameraPreview2::CameraPreview2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CameraPreview2)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    axle_camera   = new AxleCamera();
    display_timer = new QTimer();

    connect(display_timer, SIGNAL(timeout()), this, SLOT(Display()));

    axle_camera->start();

    if(axle_camera->OpenCamera())
    {
        axle_camera->StartPreview();
        display_timer->start(10);
    }
    else
    {
        ui->PreviewStatus->setText("开始");
        ui->label->setText("同轴相机打开失败");
    }

    qDebug("CameraPreview2 Init");
}

CameraPreview2::~CameraPreview2()
{
    axle_camera->quit_task();
    axle_camera->quit();
    axle_camera->wait(2000);
    display_timer->stop();
    delete axle_camera;
    delete display_timer;
    delete ui;
    qDebug("CameraPreview2 Quit");
}

void CameraPreview2::on_Quit_clicked()
{
    close();
}

void CameraPreview2::Display()
{
    display_img = axle_camera->getImage();
    display_img.scaled(ui->label->size(), Qt::KeepAspectRatio);
    ui->label->setPixmap(QPixmap::fromImage((display_img)));
}

void CameraPreview2::on_PreviewStatus_clicked()
{

}

void CameraPreview2::on_Captrue_clicked()
{

}

void CameraPreview2::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}
