#ifndef MCUCONTROL_H
#define MCUCONTROL_H

#include <QSerialPort>
#include <QMutex>
#include <QSerialPortInfo>
#include <QTimer>
#include <QList>

#define  BlowDataNumber  660

struct Coordinates
{
    signed short x_pos;
    signed short y_pos;
    signed short z_pos;
};

struct Agetimes
{
    signed short x_count;
    signed short y_count;
    signed short z_count;
};

class McuControl : public QObject
{
    Q_OBJECT

public:
    explicit McuControl(QObject *parent = nullptr);
    ~McuControl();

    bool SerialPortOpen(const QString &portName,int baudRate,QSerialPort::DataBits dataBits,
                                                             QSerialPort::Parity   parity,
                                                             QSerialPort::StopBits stopBits);
    signed char ResetMcu();
    signed char GetMcuVersion(QString* m_str, int msecs);
    signed char MotorAllInit(int msecs);
    signed char MotorInit(unsigned char index, int msecs);
    signed char MotorErrorRead(int msecs);
    signed char MotorAgetimesGet(int msecs);
    signed char MotorMoveLStop(unsigned char index);
    signed char MotorMoveL(unsigned char index, unsigned short speed, unsigned int steps);
    signed char MotorMove(unsigned char index, unsigned char dir, unsigned short speed, unsigned int steps);
    signed char MotorMove_block(unsigned char index, unsigned char dir, unsigned short speed, unsigned int steps, int msecs);
    signed char MotorOverReset(int msecs);
    signed char GetPosition(int msecs);
    signed char PumpOnlyBlow(int blow_times);
    signed char PumpBlowGetData(int blow_times, float* max_voltage, int msecs);
    signed char FanSwitch(int status, int msecs);
    void        msleep(int msec);

    struct Coordinates Position;
    struct Agetimes    MoveCount;
    unsigned short     BlowData[BlowDataNumber] = {0};

protected:
    void SerialPortClose();
    unsigned short CRC16(const unsigned char* data, unsigned short length);

    signed short time_count = 0;

private:
    QSerialPort serialport;
    QTimer      m_timer;

    QByteArray  data;
    QMutex      Mutex;

private slots:
    void DataReceive();

};

#endif // YMODEMMASTER_H
