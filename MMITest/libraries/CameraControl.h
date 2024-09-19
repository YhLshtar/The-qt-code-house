/*
*Copyright  Camyu Technologies Co., Ltd. 1998-2011. All rights reserved.
*File name:CameraControl.h
*Author:kk         Version:v0.0.1    Date:20231219
*Description:��������࣬ʵ�������������(�������á�������ȡ)��
*Others:�ޡ�
*History:
*        1.20231219 v0.0.1��
*          ��ʼ������
*/
#pragma once
#include <string>
#include <stdlib.h>
#include <stdint.h>
class CameraControl
{
public:
    /**
    * ���캯��.
    * ���ʼ����ָ��i2c�豸����ӻ���ַ.
    * @param[in]   i2c_no i2c�豸��.
    * @param[out]  slave  i2c�ӻ���ַ.
    * @retval  ��
    */
    CameraControl(int i2c_no,int slave);
    ~CameraControl();
public:
    /**
    * ���豸.
    * ���ݹ��캯�����ݵ�I2C�豸�źʹӻ���ַ��I2C�������������ôӻ���ַ.
    * @param[in]   ��.
    * @param[out]  ��.
    * @retval  true     �ɹ�
    * @retval  false    ����
    */
    bool OpenDev();
    /**
    * �ر��豸.
    * �ر�I2C������.
    * @param[in]   ��.
    * @param[out]  ��.
    * @retval  ��
    */
    void CloseDev();
    /**
    * ��������.
    * ��������Ĳ���.
    * @param[in]   cmd  �������������.
    * @param[in]   data �����������.
    * @param[out]  ��.
    * @retval  true     �ɹ�
    * @retval  false    ʧ��
    */
    bool WriteCommand(uint32_t cmd, uint32_t data);
    /**
    * ������ȡ.
    * ��ȡ�������ֵ.
    * @param[in]   cmd      �������������.
    * @param[in]   timeout  ��ȡ������Ӧ��ʱʱ��,��λ[ms].
    * @param[out]  data     ����������ݶ�ȡ����.
    * @retval      true     �ɹ�
    * @retval      false    ����
    */
    bool ReadCommand(uint32_t cmd, uint32_t &data, int timeout);
private:
    int m_i2c_no;
    int m_i2c_file;
    int m_slave;
};

