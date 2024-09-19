#ifndef CAMERAPREVIEW1_H
#define CAMERAPREVIEW1_H

#include <QWidget>
#include <QTimer>
#include "include/AerialCamera.h"

namespace Ui {
class CameraPreview1;
}

class CameraPreview1 : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPreview1(QWidget *parent = nullptr);
    ~CameraPreview1();

protected:
    void msleep(int msec);

protected:
    QImage display_img;

private slots:
    void Display();

    void on_Quit_clicked();
    void on_PreviewStatus_clicked();
    void on_Captrue_clicked();
    void on_Resolution_clicked();
    void on_Write_clicked();

    void on_pushButton1_clicked();
    void on_pushButton2_clicked();

private:
    Ui::CameraPreview1 *ui;

    AerialCamera* aerial_camera = nullptr;
    QTimer*       display_timer = nullptr;

    signed char   isPowerEnable = 0;
    int           current_page  = 0;
    bool          isPreview     = true;
    bool          isHResolution = true;

};

#endif // CAMERAPREVIEW1_H
