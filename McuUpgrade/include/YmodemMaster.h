#ifndef YMODEMMASTER_H
#define YMODEMMASTER_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QSerialPort>

#define  DataFrameSize    1024

struct PacketInfo
{
    unsigned short ymodelFrameLength;
    unsigned int   PacketHead_finsh;
    unsigned int   PacketData_finsh;
    unsigned int   PacketTail_finsh;
    unsigned int   error_count;
};

class YmodemMaster : public QObject
{
    Q_OBJECT

public:
    explicit YmodemMaster();
    ~YmodemMaster();

    signed char SlaveConnect(const QString &portName, int baudRate,
                             QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits);
    signed char SlaveCheck();
    signed char SlaveLoadBoot();
    void        SetUpgradeFile(const QString &name);
    signed char StartUpgrade();
    void        StopUpgrade();
    signed int  TotalPackets();
    QByteArray  m_FileStrData;

protected:
    int         m_FileSize;
    int         m_FrameSize;
    QFile       m_File;
    PacketInfo  m_PacketInfo;

    void        YmodemHead();
    void        YmodemData();
    void        YmodemTail();

    void        msleep(int msec);

private:
    signed char send_cmd(signed char cmd);
    signed char rece_data(int msecs);
    signed char send_head();
    signed char send_data();
    signed char send_tail();

    char        rece_status;
    bool        upgrade_start;
    bool        upgrade_quit;
    QByteArray  m_data;
    QByteArray  m_frameData;//128 or 1024
    QSerialPort     *m_serialport = nullptr;

private slots:
    void DataRevice();

signals:
    void SerialPortData(QString);
    void UpgradeStart();
    void UpgradeProcess(int);
    void UpgradeFinish();

};

#endif // YMODEMMASTER_H
