#ifndef AXLEVIDEOCAPTURE_H
#define AXLEVIDEOCAPTURE_H

#include <stdint.h>
#include <string>

class AxleVideoCapture
{
public:
    explicit AxleVideoCapture(const char* video_path);
    ~AxleVideoCapture();

    signed char startCapture();
    signed char stopCapture();
    unsigned char* getImage();
    bool isCameraOpen();

public:
    int height;
    int width;
    int frame_size;
    int dequeue_length;
    int dequeue_index;

private:
    std::string m_video_path;
    bool m_startCapt = false;
    int  fd_video    = -1;
    int  buf_type    =  0;
    unsigned char* g_imgbuffer = nullptr;

};

#endif // AXLEVIDEOCAPTURE_H
