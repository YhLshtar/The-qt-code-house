#ifndef MMITEST_H
#define MMITEST_H

#include <QMainWindow>
#include <QChart>
#include <QLineSeries>
#include "include/screentest.h"
#include "include/camerapreview1.h"
#include "include/camerapreview2.h"
#include "include/AxleCamera.h"
#include "include/McuControl.h"

QT_CHARTS_USE_NAMESPACE
QT_BEGIN_NAMESPACE
namespace Ui { class MMITest; }
QT_END_NAMESPACE

class MMITest : public QMainWindow
{
    Q_OBJECT

public:
    MMITest(QWidget *parent = nullptr);
    ~MMITest();

private slots:
    void on_Clear_clicked();
    void on_Quit_clicked();
    void on_Trumpet1_clicked();
    void on_CheckIp_clicked();
    void on_ScreenTest1_clicked();
    void on_PSensor_clicked();
    void on_ReadTemperature_clicked();

    void on_pushButton1_clicked();
    void on_pushButton2_clicked();

    void on_pushButton23_clicked();
    void on_pushButton24_clicked();
    void on_pushButton30_clicked();
    void on_pushButton31_clicked();
    void on_pushButton32_clicked();
    void on_pushButton33_clicked();

    void on_pushButton21_clicked();
    void on_pushButton22_clicked();

    void on_pushButton35_clicked();
    void on_pushButton36_clicked();

    void on_McuReset_clicked();
    void on_McuVersion_clicked();
    void on_Pumpup_clicked();

    void on_ReadPressure_clicked();
    void on_PumpBlow_clicked();
    void on_PumpOnlyBlow_clicked();

    void on_GetPosition_clicked();
    void on_MotorStandby_clicked();
    void on_XYZMotorInit_clicked();
    void on_XMotorInit_clicked();
    void on_YMotorInit_clicked();
    void on_ZMotorInit_clicked();
    void on_XMoveLeft_clicked();
    void on_XMoveRight_clicked();
    void on_YMoveUp_clicked();
    void on_YMoveDown_clicked();
    void on_ZMoveForward_clicked();
    void on_ZMoveBackWard_clicked();
    void on_Fan_clicked();

    void on_AerialCameraPreview_clicked();
    void on_CoaxialCamera_clicked();

    void on_pushButton26_clicked();

    void on_pushButton3_clicked();
    void on_pushButton4_clicked();

protected:
    void msleep(int msec);

    int  m_Pressure     = 240;
    int  blow_times     = 3800;
    int  LineEditStatus = 0;

private:
    Ui::MMITest     *ui;
    ScreenTest      *screen_test     = nullptr;
    CameraPreview1  *camera_preview1 = nullptr;
    CameraPreview2  *camera_preview2 = nullptr;
    McuControl      *mcu_control     = nullptr;
    QTimer          *m_timer         = nullptr;
    QChart          *chart           = nullptr;
    QLineSeries     *series          = nullptr;

    signed char    courrent_page    = 0;
    signed char    courrent_page2   = 0;
    char           fan_flag         = 255;

};
#endif // MMITEST_H
