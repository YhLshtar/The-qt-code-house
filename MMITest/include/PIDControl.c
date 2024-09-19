#include "PIDControl.h"

#define NB -3
#define NM -2
#define NS -1
#define ZO  0
#define PS  1
#define PM  2
#define PB  3

signed char KpMatrix[7][7]={{PB,PB,PM,PM,PS,ZO,ZO},
	                        {PB,PB,PM,PS,PS,ZO,NS},
						    {PM,PM,PM,PS,ZO,NS,NS},
	                        {PM,PM,PS,ZO,NS,NM,NM},
	                        {PS,PS,ZO,NS,NS,NM,NM},
	                        {PS,ZO,NS,NM,NM,NM,NB},
	                        {ZO,ZO,NM,NM,NM,NB,NB}};
signed char KiMatrix[7][7]={{NB,NB,NM,NM,NS,ZO,ZO},
	                        {NB,NB,NM,NS,NS,ZO,ZO},
						    {NB,NM,NS,NS,ZO,PS,PS},
	                        {NM,NM,NS,ZO,PS,PM,PM},
	                        {NM,NS,ZO,PS,PS,PM,PB},
	                        {ZO,ZO,PS,PS,PM,PB,PB},
	                        {ZO,ZO,PS,PM,PM,PB,PB}};
signed char KdMatrix[7][7]={{PS,NS,NB,NB,NB,NM,PS},
	                        {PS,NS,NB,NM,NM,NS,ZO},
						    {ZO,NS,NM,NM,NS,NS,ZO},
	                        {ZO,NS,NS,NS,NS,NS,ZO},
	                        {ZO,ZO,ZO,ZO,ZO,ZO,ZO},
	                        {PB,NS,PS,PS,PS,PS,PB},
	                        {PB,PM,PM,PM,PS,PS,PB}};

//隶属度函数，自己定义。但是函数和定义的NB--PB的值息息相关
float FunE(float x)
{
    // if(-6.0f < x && x >= -4.0f)
    // {
    //     return -0.5*x - 2;
    // }
    // else if(-4.0f < x && x >= -2.0f)
    // {
    //     return  0.5*x + 2;
    // }
    // else if(-2.0f < x && x >= 0.0f )
    // {
    //     return -0.5*x;
    // }
    // else if( 0.0f < x && x >= 2.0f )
    // {
    //     return  0.5*x;
    // }
    // else if( 2.0f < x && x >= 4.0f )
    // {
    //     return -0.5*x + 2;
    // }
    // else if( 4.0f < x && x >= 6.0f )
    // {
    //     return  0.5*x - 2;
    // }
    if(-3.0f < x && x >= -2.0f)
    {
        return -x - 2;
    }
    else if(-2.0f < x && x >= -1.0f)
    {
        return  x + 2;
    }
    else if(-1.0f < x && x >= 0.0f )
    {
        return -x;
    }
    else if( 0.0f < x && x >= 1.0f )
    {
        return  x;
    }
    else if( 1.0f < x && x >= 2.0f )
    {
        return -x + 2;
    }
    else if( 2.0f < x && x >= 3.0f )
    {
        return  x - 2;
    }

    return 1;
}

float FunEC(float x)
{
    // if(-6.0f < x && x >= -4.0f)
    // {
    //     return  0.5*x + 3;
    // }
    // else if(-4.0f < x && x >= -2.0f)
    // {
    //     return -0.5*x - 1;
    // }
    // else if(-2.0f < x && x >= 0.0f )
    // {
    //     return  0.5*x + 1;
    // }
    // else if( 0.0f < x && x >= 2.0f )
    // {
    //     return -0.5*x + 1;
    // }
    // else if( 2.0f < x && x >= 4.0f )
    // {
    //     return  0.5*x - 1;
    // }
    // else if( 4.0f < x && x >= 6.0f )
    // {
    //     return -0.5*x + 3;
    // }
    if(-3.0f < x && x >= -2.0f)
    {
        return  x + 3;
    }
    else if(-2.0f < x && x >= -1.0f)
    {
        return -x - 1;
    }
    else if(-1.0f < x && x >= 0.0f )
    {
        return  x + 1;
    }
    else if( 0.0f < x && x >= 1.0f )
    {
        return -x + 1;
    }
    else if( 1.0f < x && x >= 2.0f )
    {
        return  x - 1;
    }
    else if( 2.0f < x && x >= 3.0f )
    {
        return -x + 3;
    }

    return 0;
}

void PIDKpAdapt(struct PIDParameter* pid_para)
{
    pid_para->Kd = pid_para->Kd;
}

void PIDKiAdapt(struct PIDParameter* pid_para)
{
    pid_para->Ki = pid_para->Ki;
}

void PIDKdAdapt(struct PIDParameter* pid_para)
{
    pid_para->Kp = pid_para->Kp;
}

void PIDAdapteControl(struct PIDParameter* pid_para)
{
    float qValue[2]={0,0};        //偏差及其增量的量化值
    int indexE[2]={0,0};          //偏差隶属度索引
    float msE[2]={0,0};           //偏差隶属度
    int indexEC[2]={0,0};         //偏差增量隶属度索引
    float msEC[2]={0,0};          //偏差增量隶属度
    float qValueK[3];

    PIDKpAdapt(pid_para);
    PIDKiAdapt(pid_para);
    PIDKpAdapt(pid_para);
}

float PIDCompute(struct PIDParameter* pid_para)
{
    float Error, Value;
    
    Error = pid_para->Target_value - pid_para->Real_value;

    pid_para->Error_total += Error;
    Value = pid_para->Kp * Error + 
            pid_para->Ki * pid_para->Error_total + 
            pid_para->Kd * (Error - pid_para->Error_last);
    pid_para->Error_last = Error;

    return Value;
}

float PICompute(struct PIDParameter* pid_para)
{
    float Error, Value;
    
    Error = pid_para->Target_value - pid_para->Real_value;

    pid_para->Error_total += Error;
    Value = pid_para->Kp * Error + 
            pid_para->Ki * pid_para->Error_total;

    return Value;
}

float PDCompute(struct PIDParameter* pid_para)
{
    float Error, Value;
    
    Error = pid_para->Target_value - pid_para->Real_value;

    pid_para->Error_total += Error;
    Value = pid_para->Kp * Error +  
            pid_para->Kd * (Error - pid_para->Error_last);
    pid_para->Error_last = Error;

    return Value;
}