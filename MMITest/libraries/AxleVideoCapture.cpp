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

#include "AxleVideoCapture.h"

AxleVideoCapture::AxleVideoCapture(const char* video_path)
{
    m_video_path = video_path;
    m_startCapt  = false;
}

AxleVideoCapture::~AxleVideoCapture()
{

}

signed char AxleVideoCapture::startCapture()
{
    int ret = 0;

    do
    {
        fd_video = open(m_video_path.c_str(), O_RDWR);
        if (fd_video)
        {
            // 查询设备能力
            struct v4l2_capability  v_caps;
            memset(&v_caps, 0, sizeof(v_caps));
            ret = ioctl(fd_video, VIDIOC_QUERYCAP, &v_caps);
            if(ret < 0)
            {
                ret = -1;
                break;
            }
            // 获取当前格式
            struct v4l2_format v_sfmt;
            memset(&v_sfmt, 0, sizeof(v_sfmt));
            v_sfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
            buf_type = v_sfmt.type;
            if (ioctl(fd_video, VIDIOC_G_FMT, &v_sfmt) < 0)
            {
                ret = -2;
                break;
            }
            else
            {
                width      = v_sfmt.fmt.pix.width;
                height     = v_sfmt.fmt.pix.height;
                frame_size = v_sfmt.fmt.pix.pixelformat;
                printf("Set format: format = %d, size = %dx%d, numPlane = %d\n",
                       frame_size,
                       width,
                       height,
                       v_sfmt.fmt.pix_mp.num_planes);
            }
            // 请求缓冲区
            struct v4l2_requestbuffers  v_rqbufs;
            memset(&v_rqbufs, 0, sizeof(v_rqbufs));
            v_rqbufs.count  = 1;
            v_rqbufs.type   = v_sfmt.type;
            v_rqbufs.memory = V4L2_MEMORY_MMAP;
            ret = ioctl(fd_video, VIDIOC_REQBUFS, &v_rqbufs);
            if (ret < 0)
            {
                if (errno == EINVAL)
                {
                    std::cerr << "Video capturing or MMAP-streaming is not supported" << std::endl;
                    ret = -3;
                    break;
                }
                else
                {
                    std::cerr << "VIDIOC_REQBUFS failed: " << strerror(errno) << std::endl;
                    ret = -4;
                    break;
                }
            }
            // 查询缓冲区
            struct v4l2_buffer v_buffer;
            struct v4l2_plane  v_planes;
            memset(&v_buffer, 0, sizeof(v_buffer));
            v_buffer.type     = v_sfmt.type;
            v_buffer.memory   = V4L2_MEMORY_MMAP;
            v_buffer.length   = 1;
            v_buffer.index    = 0;
            v_buffer.m.planes = &v_planes;
            ret = ioctl(fd_video, VIDIOC_QUERYBUF, &v_buffer);
            if (ret < 0)
            {
                ret = -5;//std::cerr << "VIDIOC_QUERYBUF failed: " << strerror(errno) << std::endl;
                break;
            }
            g_imgbuffer = (unsigned char*)mmap(
                            NULL,
                            v_buffer.m.planes[0].length,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            fd_video,
                            v_buffer.m.planes[0].m.mem_offset
            );
            if (g_imgbuffer == MAP_FAILED)
            {
                ret = -6;//std::cerr << "mmap failed: " << strerror(errno) << std::endl;
                break;
            }
            // 开始捕获
            ret = ioctl(fd_video, VIDIOC_STREAMON, &buf_type);
            if(ret < 0)
            {
                ret = -7;//std::cerr << "mmap failed: " << strerror(errno) << std::endl;
                break;
            }
            m_startCapt = true;
            return 1;
        }
    }while(0);
    if (fd_video)
    {
        close(fd_video);
        fd_video = 0;
    }

    return ret;
}

signed char AxleVideoCapture::stopCapture()
{
    int ret = 0;
    if (fd_video)
    {
        struct v4l2_buffer v_buffer;
        struct v4l2_plane  v_planes;
        memset(&v_buffer, 0, sizeof(v_buffer));

        v_buffer.type     = buf_type;
        v_buffer.length   = 1;
        v_buffer.m.planes = &v_planes;
        v_buffer.memory   = V4L2_MEMORY_MMAP;
//        v_buffer.index    = 0;
//        ret = ioctl(fd_video, VIDIOC_DQBUF, &v_buffer);
//        if (ret)
//        {
//            perror("stop camera error1\n");
//        }
        ret = ioctl(fd_video, VIDIOC_QBUF, &v_buffer);
        ret = ioctl(fd_video, VIDIOC_STREAMOFF, &buf_type);
        if (ret < 0)
        {
            perror("stop camera error2\n");
        }
        munmap(g_imgbuffer, v_planes.length);
        close(fd_video);
        fd_video = 0;
    }
    m_startCapt = false;

    return 1;
}

bool AxleVideoCapture::isCameraOpen()
{
    return m_startCapt;
}

unsigned char* AxleVideoCapture::getImage()
{
    int ret = 0;
    struct v4l2_buffer v_buffer;
    struct v4l2_plane  v_planes;

    memset(&v_buffer, 0, sizeof(v_buffer));

    v_buffer.type = buf_type;
    v_buffer.length = 1;
    v_buffer.m.planes = &v_planes;
    v_buffer.memory = V4L2_MEMORY_MMAP;
//    v_buffer.index = buffer_idx;
    ret = ioctl(fd_video, VIDIOC_QBUF, &v_buffer);
    if (ret < 0)
    {
        return nullptr;
    }
    // 将缓冲区重新加入队列
    memset(&v_buffer, 0, sizeof(v_buffer));
    memset(&v_planes, 0, sizeof(v_planes));
    v_buffer.type     = buf_type;
    v_buffer.length   = 1;
    v_buffer.m.planes = &v_planes;
    v_buffer.memory   = V4L2_MEMORY_MMAP;
//    v_buffer.index    = 0;
    if ((!m_startCapt) || (fd_video <= 0))
    {
        return nullptr;
    }
    ret = ioctl(fd_video, VIDIOC_DQBUF, &v_buffer);
    if (ret < 0)
    {
        return nullptr;
    }

    return g_imgbuffer;
}
