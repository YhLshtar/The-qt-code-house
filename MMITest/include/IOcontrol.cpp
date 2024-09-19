#include <fstream>
#include <string>
#include "PIDControl.h"
#include "IOcontrol.h"

#define PUMPUP_TIMEOUT  120

using namespace std;

struct PIDParameter m_FirstPid = 
{
    .Kp = 2400.0f,
    .Ki = 0.0f,
    .Kd = 0.0f,
    .Error_total  = 0.0f,
    .Error_last   = 0.0f,
    .Target_value = 0.0f,

    .ValueMax = 40000,
    .ValueMin = 0
};

struct PIDParameter m_SecondPid =
{
    .Kp = 640.0f,
    .Ki = 300.0f,
    .Kd = 0.0f,
    .Error_total  = 0.0f,
    .Error_last   = 0.0f,
    .Target_value = 0.0f,

    .ValueMax = 40000,
    .ValueMin = 0
};

signed char StrToNum(char data)
{
    signed char mData;
    if(data >= '0' && data <= '9')
    {
        mData =  data - '0';
    }
    else if(data >= 'a' && data <= 'f')
    {
        mData =  data - 'a' + 10;
    }
    else
    {
        mData = 0;
    }
    return mData;
}

float PumpPressureControl_first()
{
    float PumpPressure;

    if(!ReadLBarometer(&PumpPressure))
    {
        return -40000.0f;
    }

    m_FirstPid.Real_value = PumpPressure;

    return PICompute(&m_FirstPid);
}

float PumpPressureControl_second()
{
    float PumpPressure;

    if(!ReadLBarometer(&PumpPressure))
    {
        return -40000.0f;
    }

    m_SecondPid.Real_value = PumpPressure;

    return PICompute(&m_SecondPid);
}
//气泵打气PWM频率25KHZ
signed char PumpUp(int target, int limit1, int limit2)
{
    unsigned int timeCount  = 0;
    m_FirstPid.Error_last   = 0;
    m_FirstPid.Error_total  = 0;
    m_FirstPid.Target_value = (float)target;
    float value = 40000.0f;

    while((value > (float)limit1) && (timeCount < PUMPUP_TIMEOUT))        //一段
    {
        value = PumpPressureControl_first();
        if(value > 500 * 80)//target value * kp
        {
            value = 40000;
        }
        else if(value <= 0)
        {
            value = 0;
        }
        SetPumpOutput((int)value);
        timeCount++;
        __LOG__("pid first: %d, count: %d\n", (int)value, timeCount);
        /*低速本身需要等待7ms，所以调节频率实际为(1000000/7000)HZ*/
    }
    SetPumpOutput(0);
    usleep(1000 * 1000);                //等1s气压稳定

    m_SecondPid.Error_last   = 0;
    m_SecondPid.Error_total  = 0;
    m_SecondPid.Target_value = (float)target;
    value = 40000.0f;

    while((value > (float)(limit2 * target)) && (timeCount < PUMPUP_TIMEOUT))       //二段
    {
        value = PumpPressureControl_second();
        if(value > 500 * 80)//target value * kp
        {
            value = 40000;
        }
        else if(value <= 0)
        {
            value = 0;
        }
        SetPumpOutput((int)value);
        timeCount++;
        __LOG__("pid second: %d, count: %d\n", (int)value, timeCount);
        /*低速本身需要等待7ms，所以调节频率实际为(1000000/7000)HZ*/
    }
    SetPumpOutput(0);
    m_SecondPid.Error_total = 0;

    if(timeCount >= PUMPUP_TIMEOUT)
    {
        return 0;
    }

    return 1;
}
/* 设置LED亮度 */
signed char LEDBrightnessSet(short Led, unsigned char value)
{
    FILE*   fd;
    unsigned char wData[3] = {(unsigned char)((Led & 0xFF00) >> 8),
                              (unsigned char)(Led & 0x00FF),
                               value};

    fd = fopen("/sys/class/tyzc/rk3588s/aw9523bLedLightSet", "w"); //打开设备节点
    if(fd == NULL)
    {
        __LOG__("open error!!!\n");
        return 0;
    }
    else
    {
        fwrite(wData, 3, 1, fd);
        fflush(fd);
    }
    fclose(fd);
    return 1;
}

signed char ReadButton(signed char* Value)
{
    int     fd;
    char    rData[2] = {0};
    ssize_t status;

    fd = open("/sys/class/rk_gpio/gpio/gpioinvalue", O_RDONLY);
    if(fd < 0)
    {
        __LOG__("open error!!!\n");
        return 0;
    }
    status = read(fd, rData, 9);
    if(status < 0)
    {
        __LOG__("read error!!!\n");
        return 0;
    }
    else
    {
//        __LOG__("%s\n", &rData[0]);
        if(rData[0] == '1')
        {
            *Value = 1;//up
        }
        else if(rData[0] == '0')
        {
            *Value = 0;//down
        }
    }
    close(fd);

    return 1;
}

signed char PumpPowerCheck(signed char* isEnable)
{
    int     fd;
    char    rData[2] = {0};
    ssize_t status;

    fd = open("/sys/class/rk_gpio/gpio/gpioAirctl", O_RDONLY);
    if(fd < 0)
    {
        __LOG__("open error!!\n");
        return 0;
    }
    status = read(fd, rData, 9);
    if(status < 0)
    {
        __LOG__("read error!!!\n");
        return 0;
    }
    else
    {
//        __LOG__("%s\n", &rData[0]);
        if(rData[0] == '1')
        {
            *isEnable = 1;
        }
        else if(rData[0] == '0')
        {
            *isEnable = 0;
        }
    }
    close(fd);

    return 1;
}
/*打开气泵电源*/
signed char PumpPowerEnable()
{
    signed char isEnable = 0;
    system(R"_(echo 1 | sudo tee /sys/class/rk_gpio/gpio/gpioAirctl)_");
    if(PumpPowerCheck(&isEnable))
    {
        if(!isEnable)
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}
/*关闭气泵电源*/
signed char PumpPowerDisable()
{
    signed char isEnable = 0;
    system(R"_(echo 0 | sudo tee /sys/class/rk_gpio/gpio/gpioAirctl)_");
    if(PumpPowerCheck(&isEnable))
    {
        if(isEnable)
        {
           return 0;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}
/*设置气泵输出大小(PWM)*/
void SetPumpOutput(int value)
{
    std::string m_str;
    m_str = "echo " + std::to_string(value) + " | sudo tee /sys/class/pwm/pwmchip0/pwm0/duty_cycle";
    system(m_str.c_str());
}
/*红外距离传感器，距离越近值越大*/
signed char ReadPSensor(int* LeftData, int* RightData)
{
    int     fd;
    char    rData[9] = {0};
    ssize_t status;

    fd = open("/sys/class/tyzc/rk3588s/tmd2635Read", O_RDONLY);
    if(fd < 0)
    {
        __LOG__("open error!!!\n");
        return 0;
    }
    status = read(fd, rData, 9);
    if(status < 0)
    {
        __LOG__("read error!!!\n");
        return 0;
    }
    else
    {
        *LeftData  = StrToNum(rData[0]) * 16 * 16 * 16 + StrToNum(rData[1]) * 16 * 16 +
                     StrToNum(rData[2]) * 16 + StrToNum(rData[3]);
        *RightData = StrToNum(rData[5]) * 16 * 16 * 16 + StrToNum(rData[6]) * 16 * 16 +
                     StrToNum(rData[7]) * 16 + StrToNum(rData[8]);
    }
    close(fd);

    return 1;
}

signed char isLBarometerLive()
{
    int     fd;
    char    rData[14] = {0};
    ssize_t status;

    fd = open("/sys/class/tyzc/rk3588s/abp2dRead", O_RDONLY);
    if(fd < 0)
    {
        __LOG__("open error!!\n");
        return 0;
    }
    status = read(fd, rData, 14);
    if(status < 0)
    {
        __LOG__("read error!!!\n");
        return 0;
    }
    else
    {
        printf("0x%x, 0x%x\n", rData[0], rData[1]);
        if(rData[0] == 'f' && rData[1] == 'f')//传感器损坏或者接触不良
        {
            close(fd);
            return -1;
        }
        else
        {

        }
    }
    close(fd);

    return 1;
}
/*获取低速气压传感器数据(气缸内压力)*/
signed char ReadLBarometer(float* airData, float* temperature)
{
    int     fd;
    char    rData[14] = {0};
    char    mData[7]  = {0};
    ssize_t status;

    fd = open("/sys/class/tyzc/rk3588s/abp2dRead", O_RDONLY);
    if(fd < 0)
    {
        __LOG__("open error!!\n");
        return 0;
    }
    status = read(fd, rData, 14);
    if(status < 0)
    {
        __LOG__("read error!!!\n");
        return 0;
    }
    else
    {
        if(rData[0] == 'f' && rData[1] == 'f')//传感器损坏或者接触不良
        {
            close(fd);
            return 0;
        }
        for (int i = 0; i < 7; ++i)//将2个字符拼接转成一个字节数据
        {
            mData[i] = 16 * (StrToNum(rData[i*2])) + (StrToNum(rData[i*2+1]));
        }
        auto press   = (float)(mData[3] + mData[2] * 256 + mData[1] * 65536);
        auto temp    = (float)(mData[6] + mData[5] * 256 + mData[4] * 65536);
        if(airData != NULL)
        {
           *airData     = (float)((((press - 1677722) * (1 - 0) / (15099494 - 1677722) + 0) * 2068.43) * 0.75);
        }
        if(temperature != NULL)
        {
           *temperature = (float)((temp * 200 /16777215) - 50);
        }
    }
    close(fd);

    return 1;
}

signed char CheckCameraPower(signed char* isEnable)
{
    int     fd;
    char    rData[2] = {0};
    ssize_t status;

    fd = open("/sys/class/rk_gpio/gpio/gpioCampwren", O_RDONLY);
    if(fd < 0)
    {
        __LOG__("open error!!!\n");
        return 0;
    }
    status = read(fd, rData, 9);
    if(status < 0)
    {
        __LOG__("read error!!!\n");
        return 0;
    }
    else
    {
//        __LOG__("%s\n", &rData[0]);
        if(rData[0] == '1')
        {
            *isEnable = 1;
        }
        else if(rData[0] == '0')
        {
            *isEnable = 0;
        }
    }
    close(fd);

    return 1;
}
/*打开面阵相机电源*/
signed char CameraPowerEnable()
{
    signed char isEnable = 0;
    system(R"_(echo 1 | sudo tee /sys/class/rk_gpio/gpio/gpioCampwren)_");
    if(CheckCameraPower(&isEnable))
    {
        if(!isEnable)
        {
           return 0;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}
/*关闭面阵相机电源*/
signed char CameraPowerDisable()
{
    signed char isEnable = 0;
    system(R"_(echo 0 | sudo tee /sys/class/rk_gpio/gpio/gpioCampwren)_");
    if(CheckCameraPower(&isEnable))
    {
        if(isEnable)
        {
           return 0;
        }
    }
    else
    {
        return 0;
    }

    return 1;
}

signed char CheckSerialPort(char* port)
{
    int     fd;

    fd = open(port, O_WRONLY);
    if(fd < 0)
    {
        __LOG__("open error!!\n");
        return 0;
    }

    close(fd);

    return 1;
}
