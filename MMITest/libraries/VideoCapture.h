/*
*Copyright ? Camyu Technologies Co., Ltd. 1998-2011. All rights reserved.
*File name:VideoCapture.h
*Author:kk         Version:v0.0.1    Date:20231219
*Description:���ͼ��ɼ��࣬ʵ�����ͼ��ɼ���
*Others:�ޡ�
*History:
*        1.20231219 v0.0.1��
*          ��ʼ������
*/
#pragma once
#include <stdint.h>
#include <string>
#define BUFF_COUNT 10

class VideoCapture
{
public:
    /**
    * ���캯��.
    * ���ʼ����ָ������豸.
    * @param[in]   video_path ����豸·��.
    * @retval  ��
    */
    VideoCapture(const char*video_path);
    ~VideoCapture();

public:
    /**
    * �����ɼ�.
    * ���ݹ��캯��ָ��������豸�����òɼ�����������ͼ��ɼ�.
    * @param[in]   ��.
    * @param[out]  ��.
    * @retval  true     �ɹ�
    * @retval  false    ����
    */
    bool startCapture();
    /**
    * ֹͣ�ɼ�.
    * �ر�����豸��ֹͣͼ��ɼ�.
    * @param[in]   ��.
    * @param[out]  ��.
    * @retval  ��
    */
    void stopCapture();
    bool isCameraOpen();
    /**
    * �ɼ�ͼ��.
    * �Ӳɼ����л�ȡһ��ͼ�񣬷���ͼ���ַ.
    * @param[in]    ��.
    * @param[out]   ��.
    * @retval  >0   �ɹ�������ͼ���ַ.
    * @retval  0    ��ȡͼ��ʧ��.
    */
    uint8_t* getImage();
    /**
    * �ͷ�ͼ��.
    * �����Ӳɼ����л�ȡ��ͼ�񣬽�ͼ�����·ŵ��ɼ�������ȥ.
    * @param[in]    ��.
    * @param[out]   ��.
    * @retval       ��.
    */
    void putImage(uint8_t*);

public:
    int height;
    int width;
    int frame_size;
    int dequeue_length;
    int dequeue_index;

private:
    std::string m_video_path;
    bool m_startCapt = false;
    int fd_video = 0;
    uint8_t* g_imgbuffers[BUFF_COUNT];
    int buf_type;
};

