#ifndef __PIDCONTROL_H
#define __PIDCONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

struct PIDParameter
{
    float Kp;
    float Ki;
    float Kd;
    float Error_total;
    float Error_last;
    float Real_value;
    float Target_value;

    int   ValueMax;//PID计算后的最大值(限幅)
    int   ValueMin;//PID计算后的最小值(限幅) 
};

float PIDCompute(struct PIDParameter* pid_para);
float PICompute(struct PIDParameter* pid_para);
float PDCompute(struct PIDParameter* pid_para);

#ifdef __cplusplus
}
#endif

#endif /* __YMODEM_H */