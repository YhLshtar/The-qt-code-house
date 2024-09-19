/*
*Copyright  Camyu Technologies Co., Ltd. 1998-2011. All rights reserved.
*File name:CameraControl.h
*Author:kk         Version:v0.0.1    Date:20231219
*Description:相机控制类，实现相机参数控制(参数配置、参数读取)。
*Others:无。
*History:
*        1.20231219 v0.0.1：
*          初始创建。
*/
#pragma once
#include <string>
#include <stdlib.h>
#include <stdint.h>
class CameraControl
{
public:
    /**
    * 构造函数.
    * 类初始化，指定i2c设备号与从机地址.
    * @param[in]   i2c_no i2c设备号.
    * @param[out]  slave  i2c从机地址.
    * @retval  无
    */
    CameraControl(int i2c_no,int slave);
    ~CameraControl();
public:
    /**
    * 打开设备.
    * 根据构造函数传递的I2C设备号和从机地址打开I2C控制器，并配置从机地址.
    * @param[in]   无.
    * @param[out]  无.
    * @retval  true     成功
    * @retval  false    错误
    */
    bool OpenDev();
    /**
    * 关闭设备.
    * 关闭I2C控制器.
    * @param[in]   无.
    * @param[out]  无.
    * @retval  无
    */
    void CloseDev();
    /**
    * 参数配置.
    * 配置相机的参数.
    * @param[in]   cmd  相机参数命令码.
    * @param[in]   data 相机参数数据.
    * @param[out]  无.
    * @retval  true     成功
    * @retval  false    失败
    */
    bool WriteCommand(uint32_t cmd, uint32_t data);
    /**
    * 参数读取.
    * 读取相机参数值.
    * @param[in]   cmd      相机参数命令码.
    * @param[in]   timeout  读取参数响应超时时间,单位[ms].
    * @param[out]  data     相机参数数据读取返回.
    * @retval      true     成功
    * @retval      false    错误
    */
    bool ReadCommand(uint32_t cmd, uint32_t &data, int timeout);
private:
    int m_i2c_no;
    int m_i2c_file;
    int m_slave;
};

