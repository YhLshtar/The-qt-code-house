#include <QDebug>
#include <QTime>
#include "include/IOcontrol.h"
#include "camerapreview1.h"
#include "ui_camerapreview1.h"

CameraPreview1::CameraPreview1(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CameraPreview1)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);

    aerial_camera = new AerialCamera();
    display_timer = new QTimer();

    ui->Captrue->setEnabled(false);
    ui->Resolution->setEnabled(false);
    ui->Write->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(0);

    connect(display_timer, SIGNAL(timeout()), this, SLOT(Display()));

    aerial_camera->start();

    CheckCameraPower(&isPowerEnable);
    if(isPowerEnable && aerial_camera->OpenCamera())
    {
        if(aerial_camera->SetResolution(0))
        {
            unsigned int Gian, Roix, Roiy, Exposure;
            msleep(100);
            aerial_camera->ReadGain(Gian);
            aerial_camera->ReadROI(Roix, Roiy);
            aerial_camera->ReadExposure(Exposure);
            ui->lineEdit1->setText(QString::number(Gian));
            ui->lineEdit2->setText(QString::number(Roix));
            ui->lineEdit3->setText(QString::number(Roiy));
            ui->lineEdit4->setText(QString::number(Exposure));
            aerial_camera->StartPreview();
            display_timer->start(33);
        }
        else
        {
            qDebug("Set Resolution Error!~~");
        }
    }
    else
    {
        ui->PreviewStatus->setText("开始");
        ui->label->setText("RX相机打开失败");
    }

    qDebug("CameraPreview1 Init");
}

CameraPreview1::~CameraPreview1()
{
    aerial_camera->quit_task();
    aerial_camera->quit();
    aerial_camera->wait(2000);
    display_timer->stop();
    delete aerial_camera;
    delete display_timer;
    delete ui;
    qDebug("CameraPreview1 Quit");
}

void CameraPreview1::on_Quit_clicked()
{
    close();
}

void CameraPreview1::on_PreviewStatus_clicked()
{
    if(isPreview)
    {
        if(isPowerEnable)
        {
            aerial_camera->StopPreview();
            display_timer->stop();
            isPreview = false;
            ui->Captrue->setEnabled(true);
            ui->Resolution->setEnabled(true);
            ui->Write->setEnabled(true);
            ui->PreviewStatus->setText("开始");
        }
    }
    else
    {
        if(isPowerEnable)
        {
            aerial_camera->StartPreview();
            display_timer->start(50);
            isPreview = true;
            ui->Captrue->setEnabled(false);
            ui->Resolution->setEnabled(false);
            ui->Write->setEnabled(false);
            ui->PreviewStatus->setText("暂停");
        }
    }
}

void CameraPreview1::on_Captrue_clicked()
{
    aerial_camera->SaveImage();
}

void CameraPreview1::Display()
{
    display_img = aerial_camera->getImage();
    display_img.scaled(ui->label->size(), Qt::KeepAspectRatio);
    ui->label->setPixmap(QPixmap::fromImage((display_img)));
}

void CameraPreview1::on_Resolution_clicked()
{
    ui->Resolution->setEnabled(false);
    QCoreApplication::processEvents();
    if(isHResolution)
    {
        isHResolution = false;
        aerial_camera->SetResolution(1);
        ui->Resolution->setText("高分辨率");
    }
    else
    {
        isHResolution = true;
        aerial_camera->SetResolution(0);
        ui->Resolution->setText("低分辨率");
    }
    QCoreApplication::processEvents();
    ui->Resolution->setEnabled(true);
}

void CameraPreview1::on_pushButton1_clicked()
{
    current_page--;
    if(current_page <= -1)
    {
        current_page = 1;
    }
    this->ui->stackedWidget->setCurrentIndex(current_page);
}

void CameraPreview1::on_pushButton2_clicked()
{
    current_page++;
    if(current_page > 1)
    {
        current_page = 0;
    }
    this->ui->stackedWidget->setCurrentIndex(current_page);
}

void CameraPreview1::on_Write_clicked()
{
    ui->Write->setEnabled(false);
    int gain     = ui->lineEdit1->text().toInt();
    int roix     = ui->lineEdit2->text().toInt();
    int roiy     = ui->lineEdit3->text().toInt();
    int exposure = ui->lineEdit4->text().toInt();
    if(!aerial_camera->SetGain(gain) ||
       !aerial_camera->SetROI(roix, roiy) ||
       !aerial_camera->SetExposure(exposure))
    {
        ui->label_5->setText("Error");
    }
    else
    {

    }
    msleep(250);
    ui->Write->setEnabled(true);
}

void CameraPreview1::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}

