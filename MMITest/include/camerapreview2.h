#ifndef CAMERAPREVIEW2_H
#define CAMERAPREVIEW2_H

#include <QWidget>
#include <QTimer>
#include "include/AxleCamera.h"

namespace Ui {
class CameraPreview2;
}

class CameraPreview2 : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPreview2(QWidget *parent = nullptr);
    ~CameraPreview2();

protected:
    void msleep(int msec);

    QImage display_img;

private slots:
    void Display();

    void on_Quit_clicked();
    void on_PreviewStatus_clicked();
    void on_Captrue_clicked();

private:
    Ui::CameraPreview2 *ui;

    AxleCamera* axle_camera   = nullptr;
    QTimer*     display_timer = nullptr;

};

#endif // CAMERAPREVIEW2_H
