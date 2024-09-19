#include "VideoCapture.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>

VideoCapture::VideoCapture(const char* video_path)
{
    m_video_path = video_path;
    m_startCapt = false;
}

VideoCapture::~VideoCapture() {}

bool VideoCapture::startCapture()
{
    int ret = 0;
    do {
        fd_video = open(m_video_path.c_str(), O_RDWR);
        if (fd_video)
        {
            struct v4l2_capability v_caps;
            ret = ioctl(fd_video, VIDIOC_QUERYCAP, &v_caps);
            if (ret < 0)
            {
                perror("");
                break;
            }
            // ö���豸��ʽ
            buf_type = (v_caps.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
                               ? V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
                               : V4L2_BUF_TYPE_VIDEO_CAPTURE;
            struct v4l2_fmtdesc v_fmt;
            struct v4l2_frmsizeenum v_fsize;
            int fmt_cnt = 0;
            do {
                v_fmt.index = fmt_cnt;
                v_fmt.type = buf_type;
                ret = ioctl(fd_video, VIDIOC_ENUM_FMT, &v_fmt);
                if (ret != 0)
                {
//                    printf("1\n");
                    break;
                }
                v_fsize.index = 0;
                v_fsize.type = buf_type;
                v_fsize.pixel_format = v_fmt.pixelformat;
                ret = ioctl(fd_video, VIDIOC_ENUM_FRAMESIZES, &v_fsize);
                if (ret != 0)
                {
//                    printf("2\n");
                    break;
                }
                fmt_cnt++;
                printf("%s\r\n", v_fmt.description);
            } while (1);
            // ���òɼ�����Ƶ��ʽ
            struct v4l2_format v_sfmt;
            memset(&v_sfmt, 0, sizeof(v_sfmt));
            v_sfmt.type = buf_type;
            v_sfmt.fmt.pix_mp.width = v_fsize.stepwise.max_width;
            v_sfmt.fmt.pix_mp.height = v_fsize.stepwise.max_height;
            v_sfmt.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_SRGGB8;
            v_sfmt.fmt.pix_mp.num_planes = 1;
            v_sfmt.fmt.pix_mp.plane_fmt[0].sizeimage = v_sfmt.fmt.pix_mp.width * v_sfmt.fmt.pix_mp.height;
            v_sfmt.fmt.pix_mp.plane_fmt[0].bytesperline = v_sfmt.fmt.pix_mp.width;
            ret = ioctl(fd_video, VIDIOC_S_FMT, &v_sfmt);
            if (ret)
            {
                perror("");
                break;
            }
            else
            {
                printf("Set format: format=%d, size=%dx%d, numPlane=%d\n",
                       v_sfmt.fmt.pix_mp.pixelformat,
                       v_sfmt.fmt.pix_mp.width,
                       v_sfmt.fmt.pix_mp.height,
                       v_sfmt.fmt.pix_mp.num_planes);
                width       = v_sfmt.fmt.pix_mp.width;
                height      = v_sfmt.fmt.pix_mp.height;
                frame_size  = width * height;
                printf("w: %d, h: %d, frame size: %d\n", width, height, frame_size);
            }
            // ���»�ȡ��Ƶ��ʽ
            struct v4l2_format v_gfmt;
            memset(&v_gfmt, 0, sizeof(v_gfmt));
            v_gfmt.type = buf_type;
            ret = ioctl(fd_video, VIDIOC_G_FMT, &v_gfmt);
            if (ret)
            {
                perror("");
                break;
            }
            else
            {
                printf("Capture: format=%d, size=%dx%d, numPlane=%d\n",
                       v_gfmt.fmt.pix_mp.pixelformat,
                       v_gfmt.fmt.pix_mp.width,
                       v_gfmt.fmt.pix_mp.height,
                       v_gfmt.fmt.pix_mp.num_planes);
            }
            // Ϊ�ɼ����仺��
            static struct v4l2_requestbuffers v_rqbufs;
            memset(&v_rqbufs, 0, sizeof v_rqbufs);
            v_rqbufs.count = BUFF_COUNT;
            v_rqbufs.type = buf_type;
            v_rqbufs.memory = V4L2_MEMORY_MMAP;
            ret = ioctl(fd_video, VIDIOC_REQBUFS, &v_rqbufs);
            if (ret)
            {
                perror("");
                break;
            }

            // ��videoϵͳ��ȡbuff��Ϣ
            for (int i = 0; i < BUFF_COUNT; i++) {
                struct v4l2_buffer v_buffer;
                struct v4l2_plane v_planes;
                memset(&v_buffer, 0, sizeof(v_buffer));
                v_buffer.type = buf_type;
                v_buffer.memory = V4L2_MEMORY_MMAP;
                v_buffer.length = 1;
                v_buffer.index = i;
                v_buffer.m.planes = &v_planes;
                if (ioctl(fd_video, VIDIOC_QUERYBUF, &v_buffer)) {
                    perror("");
                }
                g_imgbuffers[i] = (uint8_t*) mmap(
                        NULL,
                        v_planes.length,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        fd_video,
                        v_planes.m.mem_offset
                );
                // ��buff����ɼ�����
                ret = ioctl(fd_video, VIDIOC_QBUF, &v_buffer);
                if (ret) {
                    perror("");
                }
            }
            // �����ɼ�
            ret = ioctl(fd_video, VIDIOC_STREAMON, &buf_type);
            if (ret) {
                perror("");
                break;
            }
            m_startCapt = true;
            return true;
        }
    } while (0);
    if (fd_video) {
        close(fd_video);
        fd_video = 0;
    }
    return false;
}

void VideoCapture::stopCapture() {
    int ret = 0;
    if (fd_video)
    {
        // �ٲɼ�һ֡ͼ�����ͼ��ĳ�����Ϣ
        struct v4l2_buffer v_buffer;
        struct v4l2_plane v_planes;
        memset(&v_buffer, 0, sizeof(v_buffer));
        v_buffer.type = buf_type;
        v_buffer.length = 1;
        v_buffer.m.planes = &v_planes;
        v_buffer.memory = V4L2_MEMORY_MMAP;
        v_buffer.index = 0;
//        ret = ioctl(fd_video, VIDIOC_DQBUF, &v_buffer);
//        if (ret)
//        {
//            perror("stop camera error1\n");
//        }
        ret = ioctl(fd_video, VIDIOC_QBUF, &v_buffer);
        ret = ioctl(fd_video, VIDIOC_STREAMOFF, &buf_type);
        if (ret)
        {
            perror("stop camera error2\n");
        }
        for (int i = 0; i < BUFF_COUNT; i++)
        {
            munmap(g_imgbuffers[i], v_planes.length);
        }
        close(fd_video);
        fd_video = 0;
    }
    m_startCapt = false;
}

bool VideoCapture::isCameraOpen()
{
    return m_startCapt;
}

uint8_t* VideoCapture::getImage()
{
    struct v4l2_buffer v_buffer;
    struct v4l2_plane  v_planes;
    int ret = 0;
    memset(&v_buffer, 0, sizeof(v_buffer));
    v_buffer.type     = buf_type;
    v_buffer.length   = 1;
    v_buffer.m.planes = &v_planes;
    v_buffer.memory   = V4L2_MEMORY_MMAP;
    v_buffer.index    = 0;
    if ((!m_startCapt) || (fd_video <= 0))
    {
        return 0;
    }
    ret = ioctl(fd_video, VIDIOC_DQBUF, &v_buffer);
//    printf("...\n");
    if (ret >= 0)
    {
//        printf("dequeue length=%u, index=%u\n", v_planes.length, v_buffer.index);
        dequeue_length = v_planes.length;
        dequeue_index  = v_buffer.index;
        return g_imgbuffers[v_buffer.index];
    }

    return 0;
}

void VideoCapture::putImage(uint8_t* p_img)
{
    int buffer_idx = -1;
    for (int i = 0; i < BUFF_COUNT; i++)
    {
        if (p_img == g_imgbuffers[i])
        {
            buffer_idx = i;
            break;
        }
    }
    if (buffer_idx < 0)
    {
        return;
    }
    struct v4l2_buffer v_buffer;
    struct v4l2_plane v_planes;
    int ret = 0;
    memset(&v_buffer, 0, sizeof(v_buffer));
    v_buffer.type = buf_type;
    v_buffer.length = 1;
    v_buffer.m.planes = &v_planes;
    v_buffer.memory = V4L2_MEMORY_MMAP;
    v_buffer.index = buffer_idx;
    ret = ioctl(fd_video, VIDIOC_QBUF, &v_buffer);
    if (ret)
    {
        perror("");
    }
}
