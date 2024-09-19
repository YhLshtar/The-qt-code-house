#ifndef __IOCONTROL_H
#define __IOCONTROL_H

enum LEDType
{
    LEDR   = 0x0010,
    LEDG   = 0x0020,
    LEDRG  = 0x0030,
    LEDXY1 = 0x0040,
    LEDXY2 = 0x0080,
    LEDXY  = 0x00C0,
    LEDTX1 = 0x0100,
    LEDTX2 = 0x0200,
    LEDTX3 = 0x0400,
    LEDTX  = 0x0700,
    LEDALL = 0x07F0
};

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

signed char ReadPSensor(int* LeftData, int* RightData);
signed char ReadLBarometer(float* airData, float* temperature = NULL);
signed char isLBarometerLive();
signed char PumpPowerCheck(signed char* isEnable);
signed char CheckCameraPower(signed char* isEnable);
signed char CheckSerialPort(char* port);
signed char ReadButton(signed char* Value);

#ifdef __cplusplus
};
#endif

#define     __LOG__     printf

signed char PumpUp(int target, int limit1, int limit2);
signed char LEDBrightnessSet(short Led, unsigned char value);
void SetPumpOutput(int value);
signed char PumpPowerEnable();
signed char PumpPowerDisable();
signed char CameraPowerEnable();
signed char CameraPowerDisable();

#endif //__IOCONTROL_H
