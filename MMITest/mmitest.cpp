#include "mmitest.h"
#include "ui_mmitest.h"
#include "include/IOcontrol.h"
#include <QThread>
#include <QtConcurrent>
#include <QNetworkInterface>

MMITest::MMITest(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MMITest)
{
    ui->setupUi(this);
    qDebug()<<"Main Thread ID :"<<QThread::currentThreadId();

    mcu_control = new McuControl();
    m_timer     = new QTimer(this);
    series      = new QLineSeries(this);
    chart       = ui->charts_view->chart();

    ui->labelVersion->setText("MMITest_V01.A.003.240910");
    chart->legend()->hide();
    ui->charts_view->setRenderHint(QPainter::Antialiasing);

    for (unsigned int i = 0; i < BlowDataNumber; i++)
    {
        series->append(i, mcu_control->BlowData[i] * 0.1804);
    }

    chart->addSeries(series);   // 将系列添加到图表
    chart->createDefaultAxes(); // 基于已添加到图表的 series 来创建默认的坐标轴
    chart->axes(Qt::Vertical).first()->setRange(0, 300);

    ui->stackedWidget1->setCurrentIndex(0);
    ui->stackedWidget2->setCurrentIndex(0);

    ui->pushButton24->setEnabled(false);
    ui->pushButton26->setEnabled(false);
    ui->pushButton31->setEnabled(false);
    ui->pushButton33->setEnabled(false);
    ui->lineEdit5->setEnabled(false);

    m_timer->start(200);

    signed char isPumpPowerEnable = 0, isCameraPowerEnable = 0;
    if(PumpPowerCheck(&isPumpPowerEnable))
    {
        if(isPumpPowerEnable)
        {
            ui->pushButton21->setEnabled(false);
        }
        else
        {
            ui->pushButton22->setEnabled(false);
        }
    }
    else
    {
        ui->plainTextEdit->appendPlainText("1.TODO For Error");
    }

    if(CheckCameraPower(&isCameraPowerEnable))
    {
        if(isCameraPowerEnable)
        {
            ui->pushButton35->setEnabled(false);
        }
        else
        {
            ui->pushButton36->setEnabled(false);
        }
    }
    else
    {
        ui->plainTextEdit->appendPlainText("2.TODO For Error");
    }

    if(mcu_control->SerialPortOpen("/dev/ttyS3", 460800, QSerialPort::Data8,
                                                         QSerialPort::NoParity,
                                                         QSerialPort::OneStop))
    {
        QString version;
        if(mcu_control->GetMcuVersion(&version, 100))
        {
            if(version == "")
            {
                ui->plainTextEdit->appendPlainText("3588s未连接到MCU!");
                qDebug("3588s未连接到MCU!");
            }
            else
            {

            }
        }
        else
        {

        }
    }
    else
    {
        ui->plainTextEdit->appendPlainText("串口打开失败!");
        qDebug("3588s串口打开失败!");
        if(CheckSerialPort("/dev/ttyS3"))
        {
            qDebug("串口设备存在");
        }
        else
        {
            qDebug("权限不足, 找不到串口设备");
        }
    }
}

MMITest::~MMITest()
{
    m_timer->stop();
    delete m_timer;
//    delete chart;
    delete series;
    delete mcu_control;
    delete ui;
    qDebug("MMITest Main Thread Quit");
}

void MMITest::on_Clear_clicked()
{
    ui->plainTextEdit->clear();
}

void MMITest::on_Quit_clicked()
{
    close();
}

void MMITest::on_pushButton1_clicked()
{
    courrent_page--;
    if(courrent_page <= -1)
    {
        courrent_page = 3;
    }
    this->ui->stackedWidget1->setCurrentIndex(courrent_page);
}

void MMITest::on_pushButton2_clicked()
{
    courrent_page++;
    if(courrent_page >= 4)
    {
        courrent_page = 0;
    }
    this->ui->stackedWidget1->setCurrentIndex(courrent_page);
}

void MMITest::on_pushButton3_clicked()
{
    courrent_page2--;
    if(courrent_page2 <= -1)
    {
        courrent_page2 = 1;
    }
    this->ui->stackedWidget2->setCurrentIndex(courrent_page2);
}

void MMITest::on_pushButton4_clicked()
{
    courrent_page2++;
    if(courrent_page2 >= 2)
    {
        courrent_page2 = 0;
    }
    this->ui->stackedWidget2->setCurrentIndex(courrent_page2);
}

void MMITest::on_CheckIp_clicked()
{
    QString str;
    unsigned char find_flag = 0;
//    QString str = "主机名称：" + QHostInfo::localHostName() + "\n";
    /* 获取所有的网络接口，QNetworkInterface类提供主机的IP地址和网络接口的列表 */
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    /* 遍历list */
    foreach (QNetworkInterface interface, list)
    {
//        str+= "网卡设备:" + interface.name() + "\n";
        if( interface.name() == "eth0" )
        {
           str = "MAC地址:  " + interface.hardwareAddress();
        }
        /* QNetworkAddressEntry类存储IP地址子网掩码和广播地址 */
        QList<QNetworkAddressEntry> entryList = interface.addressEntries();
        /* 遍历entryList */
        foreach (QNetworkAddressEntry entry, entryList)
        {
            /* 过滤IPv6地址，只留下IPv4 */
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
//                str += "IP 地址:" + entry.ip().toString() + "\n";
//                str+= "子网掩码:" + entry.netmask().toString() + "\n";
//                str+= "广播地址:" + entry.broadcast().toString() + "\n";
                if(interface.name() == "wlan0")
                {
                    ui->plainTextEdit->appendPlainText("本机IP:  " + entry.ip().toString());
                    find_flag = 1;
                }
            }
        }
    }
    if(!find_flag)
    {
        ui->plainTextEdit->appendPlainText("WIFI未连接(>_<)");
    }
}

void MMITest::on_Trumpet1_clicked()
{
    static int i = 0;
    QtConcurrent::run([=]()
    {
        QString currenPath = QCoreApplication::applicationDirPath() + "/tts/tts_"+ QString::number(i) +".wav";
        QProcess p;

        p.start("killall aplay");
        p.waitForFinished();
        p.start("aplay " + currenPath);
        p.waitForFinished();
    });
}

void MMITest::on_ScreenTest1_clicked()
{
    screen_test = new ScreenTest();
}

void MMITest::on_PSensor_clicked()
{
    int Left = 0, Right = 0;

    if(ReadPSensor(&Left, &Right))
    {
        ui->plainTextEdit->appendPlainText("左 : " + QString::number(Left) + "  "
                                           "右 : " + QString::number(Right));
    }else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到PSensor设备");
    }
}

void MMITest::on_pushButton23_clicked()
{
    if(LEDBrightnessSet(LEDTX, 0))
    {
        if(LEDBrightnessSet(LEDTX1, 6))
        {
            ui->pushButton23->setEnabled(false);
            ui->pushButton24->setEnabled(true);
        }
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到引导灯设备");
    }
}

void MMITest::on_pushButton24_clicked()
{
    if(LEDBrightnessSet(LEDTX, 0))
    {
        ui->pushButton23->setEnabled(true);
        ui->pushButton24->setEnabled(false);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到引导灯设备");
    }
}

void MMITest::on_pushButton30_clicked()
{
    if(LEDBrightnessSet(LEDXY, 140))
    {
        ui->pushButton30->setEnabled(false);
        ui->pushButton31->setEnabled(true);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到补光灯设备");
    }
}

void MMITest::on_pushButton31_clicked()
{
    if(LEDBrightnessSet(LEDXY, 0))
    {
        ui->pushButton30->setEnabled(true);
        ui->pushButton31->setEnabled(false);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到补光灯设备");
    }
}

void MMITest::on_pushButton32_clicked()
{
    if(LEDBrightnessSet(LEDTX, 255))
    {
        ui->pushButton32->setEnabled(false);
        ui->pushButton33->setEnabled(true);
//        ui->pushButton26->setEnabled(true);
//        ui->lineEdit5->setEnabled(true);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到TX光源设备");
    }
}

void MMITest::on_pushButton33_clicked()
{
    if(LEDBrightnessSet(LEDTX, 0))
    {
        ui->pushButton32->setEnabled(true);
        ui->pushButton33->setEnabled(false);
        ui->pushButton26->setEnabled(false);
        ui->lineEdit5->setEnabled(false);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到TX光源设备");
    }
}

void MMITest::on_pushButton21_clicked()
{
    if(PumpPowerEnable())
    {
        ui->pushButton21->setEnabled(false);
        ui->pushButton22->setEnabled(true);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到气泵电源设备");
    }
}

void MMITest::on_pushButton22_clicked()
{
    if(PumpPowerDisable())
    {
        ui->pushButton21->setEnabled(true);
        ui->pushButton22->setEnabled(false);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到气泵电源设备");
    }
}

void MMITest::on_pushButton35_clicked()
{
    if(CameraPowerEnable())
    {
        ui->pushButton35->setEnabled(false);
        ui->pushButton36->setEnabled(true);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到RX相机电源设备");
    }
}

void MMITest::on_pushButton36_clicked()
{
    if(CameraPowerDisable())
    {
        ui->pushButton35->setEnabled(true);
        ui->pushButton36->setEnabled(false);
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到RX相机电源设备");
    }
}

void MMITest::on_Pumpup_clicked()
{
    ui->ReadPressure->setEnabled(false);
    ui->PumpOnlyBlow->setEnabled(false);
    ui->PumpBlow->setEnabled(false);
    ui->Pumpup->setEnabled(false);
    QCoreApplication::processEvents();
    signed char status = isLBarometerLive();
    if(status)
    {
        signed char isPumpPowerEnable = 0;
        PumpPowerCheck(&isPumpPowerEnable);
        if(isPumpPowerEnable)
        {
            m_Pressure = ui->lineEdit3->text().toUInt();
            ui->plainTextEdit->appendPlainText("打气压力 : " + QString::number(m_Pressure));
            QCoreApplication::processEvents();
            if(PumpUp(m_Pressure, 18000, 10))
            {

            }
            else
            {
                ui->plainTextEdit->appendPlainText("打气超时!");
            }
        }
        else
        {
            ui->plainTextEdit->appendPlainText("气泵未通电, 无法打气!");
        }
    }
    else if(status == -1)
    {
        ui->plainTextEdit->appendPlainText("传感器接触不良, 无法打气!");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("权限不足, 找不到低速传感器!");
    }
    QCoreApplication::processEvents();
    ui->Pumpup->setEnabled(true);
    ui->PumpBlow->setEnabled(true);
    ui->PumpOnlyBlow->setEnabled(true);
    ui->ReadPressure->setEnabled(true);
}

void MMITest::on_PumpOnlyBlow_clicked()
{
    ui->PumpOnlyBlow->setEnabled(false);
    QCoreApplication::processEvents();
    blow_times = ui->lineEdit4->text().toUInt();
    ui->plainTextEdit->appendPlainText("开阀时间 : " + QString::number(blow_times));
    signed char status = mcu_control->PumpOnlyBlow(blow_times);
    switch(status)
    {
        case -1:
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }break;
        case  1:
        {
            ui->plainTextEdit->appendPlainText("吹气");
        }break;
        default:
        {

        }break;
    }
    QCoreApplication::processEvents();
    ui->PumpOnlyBlow->setEnabled(true);
}

void MMITest::on_PumpBlow_clicked()
{
    float max_voltage;
    ui->PumpBlow->setEnabled(false);
    QCoreApplication::processEvents();
    blow_times = ui->lineEdit4->text().toUInt();
    ui->plainTextEdit->appendPlainText("开阀时间 : " + QString::number(blow_times));
    signed char status = mcu_control->PumpBlowGetData(blow_times, &max_voltage, 500);
    switch(status)
    {
        case -3:
        {
            ui->plainTextEdit->appendPlainText("通信异常!");
        }break;
        case -2:
        {
            ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
        }break;
        case -1:
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }break;
        case  1:
        {
            ui->plainTextEdit->appendPlainText("高速ADC气压数据 : " + QString::number(max_voltage * 0.1804));
            chart->removeSeries(series);
            series->clear();
            for (unsigned int i = 0; i < BlowDataNumber; i++)
            {
                series->append(i, mcu_control->BlowData[i] * 0.1804);
            }
            chart->addSeries(series);  // 将系列添加到图表
            chart->createDefaultAxes();// 基于已添加到图表的 series 来创建默认的坐标轴
            chart->axes(Qt::Vertical).first()->setRange(0, 300);
        }break;
        default:
        {

        }break;
    }
    QCoreApplication::processEvents();
    ui->PumpBlow->setEnabled(true);
}

void MMITest::on_McuReset_clicked()
{
    if(mcu_control->ResetMcu())
    {
//        ui->plainTextEdit->appendPlainText("");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("未打开串口!");
    }
}

void MMITest::on_McuVersion_clicked()
{
    QString version;
    if(mcu_control->GetMcuVersion(&version, 100))
    {
        ui->plainTextEdit->appendPlainText(version.split("\n").at(0));
    }
    else
    {
        ui->plainTextEdit->appendPlainText("未打开串口!");
    }
}

void MMITest::on_ReadPressure_clicked()
{
    float Pressure = 0.0f;
    signed char status = isLBarometerLive();
    switch(status)
    {
        case -1:
        {
            ui->plainTextEdit->appendPlainText("传感器接触不良或者已损坏!");
        }break;
        case  0:
        {
            ui->plainTextEdit->appendPlainText("权限不足, 找不到低速传感器!");
        }break;
        case  1:
        {
            if(ReadLBarometer(&Pressure, nullptr))
            {
                ui->plainTextEdit->appendPlainText("缸内压力 : " + QString::number(Pressure));
            }
            else
            {
                ui->plainTextEdit->appendPlainText("气缸压力获取失败!");
            }
        }break;
    }
}

void MMITest::on_XYZMotorInit_clicked()
{
    ui->XYZMotorInit->setEnabled(false);
    QCoreApplication::processEvents();
    signed char status = mcu_control->MotorAllInit(30 * 1000);
    switch(status)
    {
        case -4:
        case -3:
        {
            ui->plainTextEdit->appendPlainText("通信异常!");
        }break;
        case -2:
        {
            ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
        }break;
        case -1:
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }break;
        case  1:
        {
            ui->plainTextEdit->appendPlainText("三轴初始化完成");
        }break;
        default:
        {
            ui->plainTextEdit->appendPlainText("初始化异常...");
            QCoreApplication::processEvents();
            status = mcu_control->MotorErrorRead(5 * 1000);
            switch(status)
            {
                case -5:
                case -4:
                {
                    ui->plainTextEdit->appendPlainText("通信异常!");
                }break;
                case -3:
                {
                    ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
                }break;
                case -2:
                case -1:
                {
                    ui->plainTextEdit->appendPlainText("串口未打开!");
                }break;
                default:
                {
                    ui->plainTextEdit->appendPlainText("异常码为 : 0x" + QString::number(status , 16));
                }break;
            }
        }break;
    }
    QCoreApplication::processEvents();
    ui->XYZMotorInit->setEnabled(true);
}

void MMITest::on_XMotorInit_clicked()
{
    ui->XMotorInit->setEnabled(false);
    QCoreApplication::processEvents();
    signed char status = mcu_control->MotorInit(0, 20 * 1000);
    switch(status)
    {
        case -4:
        case -3:
        {
            ui->plainTextEdit->appendPlainText("通信异常!");
        }break;
        case -2:
        {
            ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
        }break;
        case -1:
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }break;
        case  1:
        {
            ui->plainTextEdit->appendPlainText("X轴初始化完成");
        }break;
        default:
        {
            ui->plainTextEdit->appendPlainText("初始化异常...");
            QCoreApplication::processEvents();
            status = mcu_control->MotorErrorRead(5 * 1000);
            switch(status)
            {
                case -5:
                case -4:
                {
                    ui->plainTextEdit->appendPlainText("通信异常!");
                }break;
                case -3:
                {
                    ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
                }break;
                case -2:
                case -1:
                {
                    ui->plainTextEdit->appendPlainText("串口未打开!");
                }break;
                default:
                {
                    ui->plainTextEdit->appendPlainText("异常码为 : 0x" + QString::number(status , 16));
                }break;
            }
        }break;
    }
    QCoreApplication::processEvents();
    ui->XMotorInit->setEnabled(true);
}

void MMITest::on_YMotorInit_clicked()
{
    ui->YMotorInit->setEnabled(false);
    QCoreApplication::processEvents();
    signed char status = mcu_control->MotorInit(1, 20 * 1000);
    switch(status)
    {
        case -4:
        case -3:
        {
            ui->plainTextEdit->appendPlainText("通信异常!");
        }break;
        case -2:
        {
            ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
        }break;
        case -1:
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }break;
        case  1:
        {
            ui->plainTextEdit->appendPlainText("Y轴初始化完成");
        }break;
        default:
        {
            ui->plainTextEdit->appendPlainText("初始化异常...");
            QCoreApplication::processEvents();
            status = mcu_control->MotorErrorRead(5 * 1000);
            switch(status)
            {
                case -5:
                case -4:
                {
                    ui->plainTextEdit->appendPlainText("通信异常!");
                }break;
                case -3:
                {
                    ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
                }break;
                case -2:
                case -1:
                {
                    ui->plainTextEdit->appendPlainText("串口未打开!");
                }break;
                default:
                {
                    ui->plainTextEdit->appendPlainText("异常码为 : 0x" + QString::number(status , 16));
                }break;
            }
        }break;
    }
    QCoreApplication::processEvents();
    ui->YMotorInit->setEnabled(true);
}

void MMITest::on_ZMotorInit_clicked()
{
    ui->ZMotorInit->setEnabled(false);
    QCoreApplication::processEvents();
    signed char status = mcu_control->MotorInit(2, 20 * 1000);
    switch(status)
    {
        case -4:
        case -3:
        {
            ui->plainTextEdit->appendPlainText("通信异常!");
        }break;
        case -2:
        {
            ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
        }break;
        case -1:
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }break;
        case  1:
        {
            ui->plainTextEdit->appendPlainText("Z轴初始化完成");
        }break;
        default:
        {
            ui->plainTextEdit->appendPlainText("初始化异常...");
            QCoreApplication::processEvents();
            status = mcu_control->MotorErrorRead(5 * 1000);
            switch(status)
            {
                case -5:
                case -4:
                {
                    ui->plainTextEdit->appendPlainText("通信异常!");
                }break;
                case -3:
                {
                    ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
                }break;
                case -2:
                case -1:
                {
                    ui->plainTextEdit->appendPlainText("串口未打开!");
                }break;
                default:
                {
                    ui->plainTextEdit->appendPlainText("异常码为 : 0x" + QString::number(status , 16));
                }break;
            }
        }break;
    }
    QCoreApplication::processEvents();
    ui->ZMotorInit->setEnabled(true);
}

void MMITest::on_MotorStandby_clicked()
{
    ui->MotorStandby->setEnabled(false);
    QCoreApplication::processEvents();
    signed char status = mcu_control->MotorOverReset(5000);
    switch(status)
    {
        case -4:
        case -3:
        {
            ui->plainTextEdit->appendPlainText("通信异常!");
        }break;
        case -2:
        {
            ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
        }break;
        case -1:
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }break;
        case  1:
        {
            ui->plainTextEdit->appendPlainText("电机待机完成");
        }break;
        default:
        {

        }break;
    }
    QCoreApplication::processEvents();
    ui->MotorStandby->setEnabled(true);
}

void MMITest::on_GetPosition_clicked()
{
    signed char status = mcu_control->GetPosition(1000);
    switch(status)
    {
        case -4:
        case -3:
        {
            ui->plainTextEdit->appendPlainText("通信异常!");
        }break;
        case -2:
        {
            ui->plainTextEdit->appendPlainText("数据校验失败, 通信异常!");
        }break;
        case -1:
        case  0:
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }break;
        case  1:
        {
            ui->plainTextEdit->appendPlainText("X: " + QString::number(mcu_control->Position.x_pos) + " "
                                               "Y: " + QString::number(mcu_control->Position.y_pos) + " "
                                               "Z: " + QString::number(mcu_control->Position.z_pos));
        }break;
        default:
        {

        }break;
    }
}

void MMITest::on_XMoveLeft_clicked()
{
    int speed = ui->lineEdit1->text().toUInt();
    int steps = ui->lineEdit2->text().toUInt();
    ui->XMoveLeft->setEnabled(false);
    QCoreApplication::processEvents();
    if(mcu_control->MotorMove(0, 1, speed, steps))
    {
        ui->plainTextEdit->appendPlainText("左移");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("串口未打开!");
    }
    QCoreApplication::processEvents();
    ui->XMoveLeft->setEnabled(true);
}

void MMITest::on_XMoveRight_clicked()
{
    int speed = ui->lineEdit1->text().toUInt();
    int steps = ui->lineEdit2->text().toUInt();
    ui->XMoveRight->setEnabled(false);
    QCoreApplication::processEvents();
    if(mcu_control->MotorMove(0, 0, speed, steps))
    {
        ui->plainTextEdit->appendPlainText("右移");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("串口未打开!");
    }
    QCoreApplication::processEvents();
    ui->XMoveRight->setEnabled(true);
}

void MMITest::on_YMoveUp_clicked()
{
    int speed = ui->lineEdit1->text().toUInt();
    int steps = ui->lineEdit2->text().toUInt();
    ui->YMoveUp->setEnabled(false);
    QCoreApplication::processEvents();
    if(mcu_control->MotorMove(1, 1, speed, steps))
    {
        ui->plainTextEdit->appendPlainText("上移");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("串口未打开!");
    }
    QCoreApplication::processEvents();
    ui->YMoveUp->setEnabled(true);
}

void MMITest::on_YMoveDown_clicked()
{
    int speed = ui->lineEdit1->text().toUInt();
    int steps = ui->lineEdit2->text().toUInt();
    ui->YMoveDown->setEnabled(false);
    QCoreApplication::processEvents();
    if(mcu_control->MotorMove(1, 0, speed, steps))
    {
        ui->plainTextEdit->appendPlainText("下移");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("串口未打开!");
    }
    QCoreApplication::processEvents();
    ui->YMoveDown->setEnabled(true);
}

void MMITest::on_ZMoveForward_clicked()
{
    int speed = ui->lineEdit1->text().toUInt();
    int steps = ui->lineEdit2->text().toUInt();
    ui->ZMoveForward->setEnabled(false);
    QCoreApplication::processEvents();
    if(mcu_control->MotorMove(2, 0, speed, steps))
    {
        ui->plainTextEdit->appendPlainText("前进");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("串口未打开!");
    }
    QCoreApplication::processEvents();
    ui->ZMoveForward->setEnabled(true);
}

void MMITest::on_ZMoveBackWard_clicked()
{
    int speed = ui->lineEdit1->text().toUInt();
    int steps = ui->lineEdit2->text().toUInt();
    ui->ZMoveBackWard->setEnabled(false);
    QCoreApplication::processEvents();
    if(mcu_control->MotorMove(2, 1, speed, steps))
    {
        ui->plainTextEdit->appendPlainText("后退");
    }
    else
    {
        ui->plainTextEdit->appendPlainText("串口未打开!");
    }
    QCoreApplication::processEvents();
    ui->ZMoveBackWard->setEnabled(true);
}

void MMITest::on_AerialCameraPreview_clicked()
{
    camera_preview1 = new CameraPreview1();
    camera_preview1->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    camera_preview1->show();
}

void MMITest::on_CoaxialCamera_clicked()
{
    //同轴调试曝光延时fps
    camera_preview2 = new CameraPreview2();
    camera_preview2->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    camera_preview2->show();
//    system(R"_(gst-launch-1.0 v4l2src device=/dev/video22 ! queue ! video/x-raw, format=NV12, width=640, height=480, framerate=120/1 ! videoconvert ! autovideosink)_");
}

void MMITest::on_pushButton26_clicked()
{
    int LEDbrightness = ui->lineEdit5->text().toUInt();
    if(LEDbrightness >= 765)
    {
        LEDbrightness = 765;
    }
    if(LEDbrightness > 255)
    {
        LEDBrightnessSet(LEDTX1, 255);
        LEDBrightnessSet(LEDTX2, LEDbrightness - 255);
        LEDBrightnessSet(LEDTX3, 0);
    }
    else if(LEDbrightness > 510)
    {
        LEDBrightnessSet(LEDTX1, 255);
        LEDBrightnessSet(LEDTX2, 255);
        LEDBrightnessSet(LEDTX3, LEDbrightness - 510);
    }
    else
    {
        LEDBrightnessSet(LEDTX1, LEDbrightness);
        LEDBrightnessSet(LEDTX2, 0);
        LEDBrightnessSet(LEDTX3, 0);
    }
}

void MMITest::on_Fan_clicked()
{
    if(fan_flag == 255)
    {
        if(mcu_control->FanSwitch(1, 250))
        {
            ui->Fan->setText("风扇关");
            fan_flag = ~fan_flag;
        }
        else
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }
    }
    else
    {
        if(mcu_control->FanSwitch(0, 250))
        {
            ui->Fan->setText("风扇开");
            fan_flag = ~fan_flag;
        }
        else
        {
            ui->plainTextEdit->appendPlainText("串口未打开!");
        }
    }
}

void MMITest::on_ReadTemperature_clicked()
{
    float Temperature = 0.0f;
    signed char status = isLBarometerLive();
    switch(status)
    {
        case -1:
        {
            ui->plainTextEdit->appendPlainText("低速传感器接触不良或者已损坏!");
        }break;
        case  0:
        {
            ui->plainTextEdit->appendPlainText("权限不足, 找不到低速传感器!");
        }break;
        case  1:
        {
            if(ReadLBarometer(nullptr, &Temperature))
            {
                ui->plainTextEdit->appendPlainText("缸内温度 : " + QString::number(Temperature));
            }
            else
            {
                ui->plainTextEdit->appendPlainText("气缸温度获取失败!");
            }
        }break;
    }
}

void MMITest::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}
