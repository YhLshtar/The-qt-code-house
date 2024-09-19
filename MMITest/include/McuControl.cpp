#include <QDebug>
#include <QThread>
#include <QtConcurrent>
#include "McuControl.h"
#include "include/IOcontrol.h"

#define  isTimerDebug 1

McuControl::McuControl(QObject *parent) : QObject(parent)
{
    qDebug("McuControl Create");
}

McuControl::~McuControl()
{
    SerialPortClose();
    qDebug("McuControl Delete");
}

unsigned short McuControl::CRC16(const unsigned char* data, unsigned short length)
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

bool McuControl::SerialPortOpen(const QString &portName,
                                int baudRate,
                                QSerialPort::DataBits dataBits,
                                QSerialPort::Parity   parity,
                                QSerialPort::StopBits stopBits)
{
    serialport.setPortName(portName);
    serialport.setBaudRate(baudRate);
    serialport.setDataBits(dataBits);
    serialport.setParity(parity);
    serialport.setStopBits(stopBits);

    if (serialport.open(QIODevice::ReadWrite))
    {
#if(isTimerDebug == 1)
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(DataReceive()));
#else
        connect(&serialport, &QSerialPort::readyRead, this, &McuControl::DataReceive);
#endif
        return true;
    }
    else
    {
        return false;
    }
}

void McuControl::SerialPortClose()
{
    if(serialport.isOpen())
    {
        serialport.close();
    }
}

void McuControl::DataReceive()
{
    time_count--;
    QByteArray data_buf = serialport.readAll();
    qDebug() << data_buf.toHex();
    data += data_buf;
}

signed char McuControl::ResetMcu()
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x15;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if(serialport.isOpen())
    {
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
            serialport.waitForBytesWritten();
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
        }
        else
        {
            qDebug("ERROR: -1");
            return -1;
        }
    }
    else
    {
        qDebug("串口没有打开！");
        return 0;
    }

    return 1;
}

signed char McuControl::GetMcuVersion(QString* m_str, int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x16;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
//            serialport.waitForBytesWritten();
            serialport.waitForReadyRead(msecs);
            QByteArray data = serialport.readAll();
            qDebug()<<"Mcu Version: "<<data;
            *m_str = data;
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug("串口没有打开！");
        return 0;
    }

    return 1;
}

signed char McuControl::MotorAllInit(int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x20;
    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
//            serialport.waitForBytesWritten();
            serialport.waitForReadyRead(msecs);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
            char* ydata = Data.data();
            if(ydata[1] != 0xEE)
            {
                if(ydata[0] == 0x01 && ydata[19] == 0x04)
                {
                    if(ydata[1] != 0x25)
                    {
                        qDebug("ERROR: -4");
                        Mutex.unlock();
                        return -4;
                    }
                    else
                    {
                        if(ydata[2] != 0x66)
                        {
                            qDebug("ERROR: default, number %x", ydata[2]);
                            Mutex.unlock();
                            return ydata[2];
                        }
                    }
                }
                else
                {
                    qDebug("ERROR: -3");
                    Mutex.unlock();
                    return -3;
                }
            }
            else
            {
                qDebug("ERROR: -2");
                Mutex.unlock();
                return -2;
            }
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug("串口没有打开！");
        return 0;
    }

    return 1;
}

signed char McuControl::MotorInit(unsigned char index, int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x04;
    buf[6] = index;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
//            serialport.waitForBytesWritten();
            serialport.waitForReadyRead(msecs);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
            char* ydata = Data.data();
            if(ydata[1] != 0xEE)
            {
                if(ydata[0] == 0x01 && ydata[19] == 0x04)
                {
                    if(ydata[1] != 0x25)
                    {
                        qDebug("ERROR: -4");
                        Mutex.unlock();
                        return -4;
                    }
                    else
                    {
                        if(ydata[2] != 0x66)
                        {
                            qDebug("ERROR: default, number %x", ydata[2]);
                            Mutex.unlock();
                            return ydata[2];
                        }
                    }
                }
                else
                {
                    qDebug("ERROR: -3");
                    Mutex.unlock();
                    return -3;
                }
            }
            else
            {
                qDebug("ERROR: -2");
                Mutex.unlock();
                return -2;
            }
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::MotorErrorRead(int msecs)
{
    signed char ret = 0;

    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x25;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
//            serialport.waitForBytesWritten();
            serialport.waitForReadyRead(msecs);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
            char* ydata = Data.data();
            if(ydata[1] != 0xEE)
            {
                if(ydata[0] == 0x01 && ydata[19] == 0x04)
                {
                    if(ydata[1] != 0x25)
                    {
                        qDebug("ERROR: -5");
                        ret = -5;
                    }
                    else
                    {
                        ret = ydata[3];
                    }
                }
                else
                {
                    qDebug("ERROR: -4");
                    ret = -4;
                }
            }
            else
            {
                qDebug("ERROR: -3");
                ret = -3;
            }
        }
        else
        {
            qDebug("ERROR: -2");
            ret = -2;
        }
    }
    else
    {
        qDebug("串口没有打开！");
        ret = -1;
    }

    Mutex.unlock();
    return ret;
}

signed char McuControl::MotorAgetimesGet(int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x23;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
//            serialport.waitForBytesWritten();
            serialport.waitForReadyRead(msecs);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
            char* ydata = Data.data();
            if(ydata[1] != 0xEE)
            {
                if(ydata[0] == 0x01 && ydata[19] == 0x04)
                {
                    if(ydata[1] != 0x23)
                    {
                        qDebug("ERROR: -4");
                        Mutex.unlock();
                        return -4;
                    }
                    else
                    {
                        if(ydata[2] != 0x66)
                        {
                            qDebug("ERROR: default, number %x", ydata[2]);
                            Mutex.unlock();
                            return ydata[2];
                        }
                        else
                        {
                            MoveCount.x_count = *((short*)(&ydata[3]));
                            MoveCount.y_count = *((short*)(&ydata[5]));
                            MoveCount.z_count = *((short*)(&ydata[7]));
                        }
                    }
                }
                else
                {
                    qDebug("ERROR: -3");
                    Mutex.unlock();
                    return -3;
                }
            }
            else
            {
                qDebug("ERROR: -2");
                Mutex.unlock();
                return -2;
            }
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::MotorMoveLStop(unsigned char index)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x22;
    buf[2] = index;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
            serialport.waitForBytesWritten(250);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::MotorMoveL(unsigned char  index,
                                   unsigned short speed,
                                   unsigned int   steps)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x21;
    buf[2] = index;
    buf[3] = 0x00;
    buf[4] = 0x20;
    memcpy(&buf[5], reinterpret_cast<char*>(&speed), 2);//三轴转速建议使用50以内不同的质数
    memcpy(&buf[7], reinterpret_cast<char*>(&steps), 4);

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
            serialport.waitForBytesWritten(250);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::MotorMove(unsigned char  index,
                                  unsigned char  dir,
                                  unsigned short speed,
                                  unsigned int   steps)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x24;
    buf[2] = index;
    buf[3] = dir;
    buf[4] = 0x20;
    memcpy(&buf[5], reinterpret_cast<char*>(&speed), 2);
    memcpy(&buf[7], reinterpret_cast<char*>(&steps), 4);

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
            serialport.waitForBytesWritten(250);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::MotorMove_block(unsigned char  index,
                                        unsigned char  dir,
                                        unsigned short speed,
                                        unsigned int   steps, int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x03;
    buf[2] = index;
    buf[3] = dir;
    buf[4] = 0x20;
    memcpy(&buf[5], reinterpret_cast<char*>(&speed), 2);
    memcpy(&buf[7], reinterpret_cast<char*>(&steps), 4);

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
//            serialport.waitForBytesWritten();
            serialport.waitForReadyRead(msecs);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
            char* ydata = Data.data();
            if(ydata[1] != 0xEE)
            {
                if(ydata[0] == 0x01 && ydata[19] == 0x04)
                {
                    if(ydata[1] != 0x03)
                    {
                        qDebug("ERROR: -4");
                        Mutex.unlock();
                        return -4;
                    }
                    else
                    {
                        if(ydata[2] != 0x66)
                        {
                            qDebug("ERROR: default, number %x", ydata[2]);
                            Mutex.unlock();
                            return ydata[2];
                        }
                    }
                }
                else
                {
                    qDebug("ERROR: -3");
                    Mutex.unlock();
                    return -3;
                }
            }
            else
            {
                qDebug("ERROR: -2");
                Mutex.unlock();
                return -2;
            }
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::MotorOverReset(int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x06;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
//            serialport.waitForBytesWritten();
            serialport.waitForReadyRead(msecs);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
            char* ydata = Data.data();
            if(ydata[1] != 0xEE)
            {
                if(ydata[0] == 0x01 && ydata[19] == 0x04)
                {
                    if(ydata[1] != 0x06)
                    {
                        qDebug("ERROR: -4");
                        Mutex.unlock();
                        return -4;
                    }
                    else
                    {
                        if(ydata[2] != 0x66)
                        {
                            qDebug("ERROR: default, number %x", ydata[2]);
                            return ydata[2];
                        }
                    }
                }
                else
                {
                    qDebug("ERROR: -3");
                    Mutex.unlock();
                    return -3;
                }
            }
            else
            {
                qDebug("ERROR: -2");
                Mutex.unlock();
                return -2;
            }
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::GetPosition(int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x18;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
//            serialport.waitForBytesWritten();
            serialport.waitForReadyRead(msecs);
            QByteArray PosData = serialport.readAll();
            qDebug()<<": "<<PosData.toHex();
            char* ydata = PosData.data();
            if(ydata[1] != 0xEE)
            {
                if(ydata[0] == 0x01 && ydata[19] == 0x04)
                {
                    if(ydata[1] != 0x18)
                    {
                        qDebug("ERROR: -4");
                        Mutex.unlock();
                        return -4;
                    }
                    else
                    {
                        if(ydata[2] != 0x66)
                        {
                            qDebug("ERROR: default, number %x", ydata[2]);
                            return ydata[2];
                        }
                        else
                        {
                            Position.x_pos = *((short*)(&ydata[4]));
                            Position.y_pos = *((short*)(&ydata[6]));
                            Position.z_pos = *((short*)(&ydata[8]));
                        }
                    }
                }
                else
                {
                    qDebug("ERROR: -3");
                    Mutex.unlock();
                    return -3;
                }
            }
            else
            {
                qDebug("ERROR: -2");
                Mutex.unlock();
                return -2;
            }
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::PumpOnlyBlow(int blow_times)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x10;
    memcpy(&buf[2], reinterpret_cast<char*>(&blow_times), 4);

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if(serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
            serialport.waitForBytesWritten();
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

signed char McuControl::PumpBlowGetData(int blow_times, float* max_voltage, int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x12;
    memcpy(&buf[2], reinterpret_cast<char*>(&blow_times), 4);

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
//        qDebug() << "Write to serial: " << sendBuf.toHex();
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);

            m_timer.start(msecs/5);
            time_count = 5;
            while(time_count > 0)
            {
                QCoreApplication::processEvents();
            }
            m_timer.stop();

            char* blow_data = data.data();
            if(blow_data[1] != 0xEE)
            {
                if(blow_data[0] == 0x01 && blow_data[BlowDataNumber * 2 + 1] == 0x04)
                {
                    for(int i = 0; i < BlowDataNumber; i++)
                    {
                        unsigned short temp = (unsigned short)((blow_data[i * 2 + 2] << 8) | blow_data[i * 2 + 1]);
                        BlowData[i]  = temp;
                        *max_voltage = (*max_voltage > temp) ? *max_voltage : temp;
                    }
                }
                else
                {
                    qDebug("ERROR: -3");
                    Mutex.unlock();
                    return -3;
                }
                data.clear();
            }
            else
            {
                qDebug("ERROR: -2");
                Mutex.unlock();
                return -2;
            }
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug("串口没有打开！");
        return 0;
    }

    return 1;
}

signed char McuControl::FanSwitch(int status, int msecs)
{
    unsigned char buf[20];

    memset(buf, 0, 20);
    buf[0] = 0x67;
    buf[1] = 0x17;
    buf[6] = status;

    unsigned short crc = CRC16(buf, 17);

    buf[18]= crc >> 8;
    buf[17]= crc & 0XFF;
    buf[19]= 0x04;

    if (serialport.isOpen())
    {
        Mutex.lock();
        QByteArray sendBuf(reinterpret_cast<char*>(buf), 20);
        if(serialport.isWritable())
        {
            serialport.write(sendBuf);
            serialport.waitForReadyRead(msecs);
            QByteArray Data = serialport.readAll();
            qDebug()<<": "<<Data.toHex();
        }
        else
        {
            qDebug("ERROR: -1");
            Mutex.unlock();
            return -1;
        }
        Mutex.unlock();
    }
    else
    {
        qDebug() << "串口没有打开！";
        return 0;
    }

    return 1;
}

void McuControl::msleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents();
    }
}
