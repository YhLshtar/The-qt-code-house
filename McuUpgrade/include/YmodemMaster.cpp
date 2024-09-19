#include <QDebug>
#include <QFileInfo>
#include <QtConcurrent>
#include "YmodemPacket.h"
#include "YmodemMaster.h"

unsigned short CRC(const unsigned char* data, unsigned short length)
{
    unsigned short crc = 0xFFFF;
    for (unsigned short i = 0; i < length; i++)
    {
        crc ^= (unsigned short)data[i] << 8; // 将数据字节移至高位并与CRC异或
        for (unsigned char j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

YmodemMaster::YmodemMaster()
{
    m_PacketInfo.ymodelFrameLength = DataFrameSize;

    m_serialport = new QSerialPort(this);

    upgrade_quit = false;

    qDebug("Create Ymodem Master");
}

YmodemMaster::~YmodemMaster()
{
    delete m_serialport;
    qDebug("Delete Ymodem Master");
}

void YmodemMaster::DataRevice()
{
    QByteArray data_buf = m_serialport->readAll();
    emit SerialPortData(data_buf);
}

signed char YmodemMaster::SlaveCheck()
{
    signed   char status = 0;
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x16;
    unsigned short crc = CRC(buf, 17);
    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
    disconnect(m_serialport, &QSerialPort::readyRead, this, &YmodemMaster::DataRevice);
    if(m_serialport->isWritable())
    {
        m_serialport->write(sendBuf);
        m_serialport->waitForReadyRead(2000);
        QByteArray data = m_serialport->readAll();
        qDebug()<<"Mcu Version: "<<data;
        if(data != "")
        {
            if(data != "C")
            {
                status = 2;
            }
            else
            {
                status = 1;
                emit
            }
        }
        else
        {
            qDebug("连接MCU失败!~~");
            return 0;
        }
    }

    connect(m_serialport, &QSerialPort::readyRead, this, &YmodemMaster::DataRevice);
    return status;
}

signed char YmodemMaster::SlaveConnect(const QString &portName, int baudRate,
                                        QSerialPort::DataBits dataBits,
                                        QSerialPort::Parity   parity,
                                        QSerialPort::StopBits stopBits)
{
    signed char status = 1;

    m_serialport->setPortName(portName);
    m_serialport->setBaudRate(baudRate);
    m_serialport->setDataBits(dataBits);
    m_serialport->setParity(parity);
    m_serialport->setStopBits(stopBits);
    if(m_serialport->open(QIODevice::ReadWrite))
    {
        if (m_serialport->isOpen())
        {
            status =  1;
        }
        else
        {
            status = -1;
        }
    }
    else
    {
        qDebug("串口打开失败!~~");
        status = 0;
    }

    connect(m_serialport, &QSerialPort::readyRead, this, &YmodemMaster::DataRevice);

    return status;
}

void YmodemMaster::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}

signed char YmodemMaster::SlaveLoadBoot()
{
    unsigned char buf[20];
    signed   char status = 1;

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x26;

    unsigned short crc = CRC(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;
    disconnect(m_serialport, &QSerialPort::readyRead, this, &YmodemMaster::DataRevice);
    if(m_serialport->isOpen())
    {
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(m_serialport->isWritable())
        {
            m_serialport->write(sendBuf);
            m_serialport->waitForReadyRead(5000);
            QByteArray Data = m_serialport->readAll();
            qDebug()<<": "<<Data.toHex();
            connect(m_serialport, &QSerialPort::readyRead, this, &YmodemMaster::DataRevice);
            char* ydata = Data.data();
            if(ydata[1] != 0xEE)
            {
                if(ydata[0] == 0x01 && ydata[19] == 0x04)
                {
                    if(ydata[1] != 0x25)
                    {
                        qDebug("ERROR: -4");
                        status = -4;
                    }
                    else
                    {
                        if(ydata[2] != 0x66)
                        {
                            qDebug("ERROR: default, number %x", ydata[2]);
                            status = ydata[2];
                        }
                    }
                }
                else
                {
                    qDebug("ERROR: -3");
                    status = -3;
                }
            }
            else
            {
                qDebug("ERROR: -2");
                status = -2;
            }
        }
        else
        {
            qDebug("ERROR: -1");
            status = -1;
        }
    }
    else
    {
        qDebug() << "串口没有打开！";
        status = 0;
    }

    return status;
}

void YmodemMaster::SetUpgradeFile(const QString &name)
{
    m_File.setFileName(name);
    QFileInfo info(name);
    m_FileSize = info.size();
    m_FrameSize = (m_FileSize % m_PacketInfo.ymodelFrameLength) ?
                  (m_FileSize / m_PacketInfo.ymodelFrameLength + 1) :
                  (m_FileSize / m_PacketInfo.ymodelFrameLength);
    //文件名 + 文件大小( + 文件最后修改时间（8进制）+ 文件模式)
    m_FileStrData  = name.split("/").last().toLocal8Bit() + '\0' +
                     QString("%1").arg(info.size()).toLocal8Bit() + '\0';
    qDebug()<<"file  size  : "<<m_FileSize;
    qDebug()<<"frame number: "<<m_FrameSize;
}

void YmodemMaster::StopUpgrade()
{
    upgrade_quit = true;
}

signed char YmodemMaster::StartUpgrade()
{
    unsigned char count = 0;

    m_PacketInfo.PacketHead_finsh = 0;
    m_PacketInfo.PacketData_finsh = 0;
    m_PacketInfo.PacketTail_finsh = 0;

    qDebug("start upgrade");
    if(m_File.open(QIODevice::ReadOnly))
    {

    }
    else
    {
        qDebug("固件打开失败, 终止升级");
        return 0;
    }
    msleep(2000);
    while(!disconnect(m_serialport, &QSerialPort::readyRead, this, &YmodemMaster::DataRevice))
    {
        QCoreApplication::processEvents();
        if(count++ > 10)
        {
            return -1;
        }
    }
    emit UpgradeStart();
    upgrade_start = false;
    upgrade_quit  = false;
    YmodemHead();
    YmodemData();
    YmodemTail();

    m_File.close();
    qDebug("Waiting...");
    m_serialport->waitForReadyRead(30*1000);
    QByteArray Data = m_serialport->readAll();
    qDebug()<<": "<<Data.toHex();
    connect(m_serialport, &QSerialPort::readyRead, this, &YmodemMaster::DataRevice);

    if(upgrade_quit)
    {
        qDebug("transfer cancel");
        return -1;
    }
    else
    {
        qDebug("transfer completed");
    }

    return 1;
}

void YmodemMaster::YmodemHead()
{
    bool y_start = false;

    while(!upgrade_quit)
    {
        rece_status = rece_data(1000);
        qDebug()<<QString::number(rece_status, 16);
        QCoreApplication::processEvents();
        switch(rece_status)
        {
            case CRC16:
            {
                if(!upgrade_start)
                {
                    if(send_head())
                    {
                        qDebug("Master Head send ok");
                        upgrade_start = true;
                    }
                    else
                    {
                        //send failed
                    }
                }
                else
                {
                    if(y_start)
                    {
                        m_PacketInfo.PacketHead_finsh = 1;
                        send_data();
                        qDebug("Master Data send ok");
                    }
                }
            }break;
            case NAK:
            {
                if(upgrade_start)
                {
                    send_head();
                    qDebug("head send again");
                }
                else
                {
                    //Transmission process error Or Receive data loss
                }
            }break;
            case ACK:
            {
                if(upgrade_start)
                {
                    if(!y_start)
                    {
                        y_start = true;
                        qDebug("Slave head rece ok");
                    }
                }
                else
                {
                    //Transmission process error Or Receive data loss
                }
            }break;
            case EOT:
            {
                upgrade_quit = true;
            }
            default:
            {

            }break;
        }
        if(m_PacketInfo.PacketHead_finsh)
        {
            break;
        }
        QCoreApplication::processEvents();
    }
}

void YmodemMaster::YmodemData()
{
    while(m_PacketInfo.PacketData_finsh < (unsigned int)m_FrameSize && (!upgrade_quit))
    {
        rece_status = rece_data(1000);
        switch(rece_status)
        {
            case ACK:
            {
                m_PacketInfo.PacketData_finsh++;
                emit UpgradeProcess(m_PacketInfo.PacketData_finsh);
                if(m_PacketInfo.PacketData_finsh < (unsigned int)m_FrameSize)
                {
                    send_data();
                    qDebug("data send");
                }
                else
                {
                    QThread::msleep(200);
                    send_cmd(EOT);
                    qDebug("Send EOT1");
                }
            }break;
            case NAK:
            {
                if(send_data())
                {
                    qDebug("data send again");
                }
                else
                {
                    //send failed
                }
            }break;
            case EOT:
            {
                upgrade_quit = true;
            }
            default:
            {

            }break;
        }
        QCoreApplication::processEvents();
    }
}

void YmodemMaster::YmodemTail()
{
    bool y_end   = false;

    while(!upgrade_quit)
    {
        rece_status = rece_data(1000);
        switch(rece_status)
        {
            case CRC16:
            {
                if(y_end)
                {
                    send_tail();
                    qDebug("tail send");
//                    emit UpgradeProcess("tail send");
                }
                else
                {

                }
            }break;
            case NAK:
            {
                send_cmd(EOT);
                qDebug("Send EOT2");
            }break;
            case ACK:
            {
                if(!y_end)
                {
                    y_end = true;
                }
                else
                {
                    qDebug("upgrade finish");
                    emit UpgradeFinish();
                    QCoreApplication::processEvents();
                    return;
                }
            }break;
            case EOT:
            {
                upgrade_quit = true;
            }
            default:
            {

            }break;
        }
        QCoreApplication::processEvents();
    }
}

signed char YmodemMaster::rece_data(int msecs)
{
    m_serialport->waitForReadyRead(msecs);
    m_data = m_serialport->readAll();
    char*  y_data = m_data.data();
    return y_data[0];
}

signed char YmodemMaster::send_cmd(signed char cmd)
{
    signed char buf[1] = {cmd};

    if (m_serialport->isOpen())
    {
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 1);
        m_serialport->write(sendBuf);
//        serialport.waitForBytesWritten();
    }
    else
    {
        qDebug("串口没有打开!");
        return 0;
    }

    return 1;
}

signed char YmodemMaster::send_head()
{
    m_frameData = head_packet(m_FileStrData, m_PacketInfo.ymodelFrameLength);
//    qDebug()<<QString(m_frameData.toHex())<< m_frameData.length();

    if (m_serialport->isOpen())
    {
        m_serialport->write(m_frameData);
//        serialport.waitForBytesWritten();
    }
    else
    {
        qDebug("串口没有打开!");
        return 0;
    }

    return 1;
}

signed char YmodemMaster::send_data()
{
    m_File.seek(m_PacketInfo.ymodelFrameLength * m_PacketInfo.PacketData_finsh);
    QByteArray file_str = m_File.read(m_PacketInfo.ymodelFrameLength);
    m_frameData = data_packet(file_str, m_PacketInfo.ymodelFrameLength);
//    qDebug()<<QString(m_frameData.toHex())<< m_frameData.length();

    if (m_serialport->isOpen())
    {
        m_serialport->write(m_frameData);
//        serialport.waitForBytesWritten();
    }
    else
    {
        qDebug("串口没有打开!");
        return 0;
    }

    return 1;
}

signed char YmodemMaster::send_tail()
{
    m_frameData = tail_packet(128);
//    qDebug()<<QString(m_frameData.toHex())<< m_frameData.length();

    if (m_serialport->isOpen())
    {
        m_serialport->write(m_frameData);
//        serialport.waitForBytesWritten();
    }
    else
    {
        qDebug("串口没有打开!");
        return 0;
    }

    return 1;
}

signed int YmodemMaster::TotalPackets()
{
    return m_FrameSize;
}
