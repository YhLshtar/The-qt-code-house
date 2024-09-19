#include "CameraControl.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* for strcasecmp() */
#include <sys/ioctl.h>
#include <sys/param.h> /* for NAME_MAX */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iostream>

using namespace std;
using namespace chrono;
static __s32 i2c_read_byte(int file) {
    union i2c_smbus_data data;
    struct i2c_smbus_ioctl_data args;
    __s32 err;

    args.read_write = I2C_SMBUS_READ;
    args.command = 0;
    args.size = 1;
    args.data = &data;

    err = ioctl(file, I2C_SMBUS, &args);
    if (err == -1) err = -errno;
    if (err < 0) {
        return 0xFF;
    }
    return data.byte & 0xFF;
}
static __s32 i2c_write(int file, __u16 slave, int size, __u8* data) {
    struct i2c_msg imsg;
    __s32 err;

    imsg.addr = slave;
    imsg.flags = 0;
    imsg.len = size;
    imsg.buf = data;
    struct i2c_rdwr_ioctl_data rdwr;

    rdwr.msgs = &imsg;
    rdwr.nmsgs = 1;
    err = ioctl(file, I2C_RDWR, &rdwr);
    if (err == -1) err = -errno;
    return err;
}
static __s32 i2c_read(int file, __u16 slave, int size, __u8* data) {
    struct i2c_msg imsg[2];
    __s32 err;

    imsg[0].addr = slave;
    imsg[0].flags = I2C_M_RD;
    imsg[0].len = size;
    imsg[0].buf = data;
    struct i2c_rdwr_ioctl_data rdwr;

    rdwr.msgs = imsg;
    rdwr.nmsgs = 1;
    err = ioctl(file, I2C_RDWR, &rdwr);
    if (err == -1) err = -errno;
    return err;
}
CameraControl::CameraControl(int i2c_no, int slave) {
    m_i2c_no = i2c_no;
    m_i2c_file = -1;
    m_slave = slave;
}

CameraControl::~CameraControl() {}

bool CameraControl::OpenDev() {
    int file, len;
    int i2cbus = m_i2c_no;
    char filename[20];
    int size = sizeof(filename);
    len = snprintf(filename, size, "/dev/i2c/%d", i2cbus);
    if (len >= (int) size) {
        fprintf(stderr, "%s: path truncated\n", filename);
        return false;
    }
    file = open(filename, O_RDWR);

    if (file < 0 && (errno == ENOENT || errno == ENOTDIR)) {
        len = snprintf(filename, size, "/dev/i2c-%d", i2cbus);
        if (len >= (int) size) {
            fprintf(stderr, "%s: path truncated\n", filename);
            return false;
        }
        file = open(filename, O_RDWR);
    }
    if (file > 0) {
        m_i2c_file = file;
    }
    ioctl(file, I2C_SLAVE, m_slave);
    return file > 0;
}

void CameraControl::CloseDev() {
    if (m_i2c_file > 0) {
        close(m_i2c_file);
    }
    m_i2c_file = 0;
}

bool CameraControl::WriteCommand(uint32_t cmd, uint32_t data) {
    struct i2c_msg imsg;
    __s32 err;
    int file = m_i2c_file;
    if (file <= 0) {
        return false;
    }
    __u8 w_cmd[] = {0x57, 0x01, 0x08, 0x00, 0x00, 0x00, 0x07};
    w_cmd[1] = (cmd >> 8) & 0xFF;
    w_cmd[2] = cmd & 0xFF;
    w_cmd[3] = (data >> 24) & 0xFF;
    w_cmd[4] = (data >> 16) & 0xFF;
    w_cmd[5] = (data >> 8) & 0xFF;
    w_cmd[6] = (data >> 0) & 0xFF;
    return i2c_write(file, m_slave, sizeof(w_cmd), w_cmd) >= 0;
}

bool CameraControl::ReadCommand(uint32_t cmd, uint32_t& data, int timeout) {
    struct i2c_msg imsg[2];
    __s32 err;
    int file = m_i2c_file;
    int timeout_flag = 0;
    if (file <= 0) {
        return false;
    }
    __u8 w_cmd[] = {0x52, 0x01, 0x08};
    __u8 r_data[] = {0x00, 0x00, 0x00, 0x00, 0x07};
    w_cmd[1] = (cmd >> 8) & 0xFF;
    w_cmd[2] = cmd & 0xFF;
    err = i2c_write(file, m_slave, sizeof(w_cmd), w_cmd);
    if (err < 0) {
        printf("i2c write failed with err=%d\n", err);
        return false;
    }
    usleep(1000 * 20);
    auto base_time = std::chrono::system_clock::now();
    //�ȴ�ͬ����
    do {
        int rbyte = i2c_read_byte(file);
        if (rbyte == 0xEB) {
            break;
        }
        auto new_time = std::chrono::system_clock::now();
        if ((new_time - base_time) > std::chrono::milliseconds(timeout)) {
            timeout_flag = 1;
            break;
        }
        usleep(1000 * 1);
    } while (1);
    if (timeout_flag) {
        printf("read command timeout");
        return false;
    }

    err = i2c_read(file, m_slave, 5, r_data);
    if (err >= 0) {
        data = ((r_data[1] << 24) | (r_data[2] << 16) | (r_data[3] << 8) |
                r_data[4]);
        return true;
    }
    printf("i2c_read failed with err=%d\n", err);
    return false;
}
