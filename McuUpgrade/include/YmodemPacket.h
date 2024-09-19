#ifndef YMODEMPACKET_H
#define YMODEMPACKET_H

#include <QByteArray>

#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)
#define FRAME_SIZE							(PACKET_SIZE + PACKET_OVERHEAD)
#define FRAME_1K_SIZE						(PACKET_1K_SIZE + PACKET_OVERHEAD)
/*	FILE_NAME和FILE_SIZE加起来不满128字节则以0x00填充剩余字节
    以下限制为稍微理想状态,实际工程按情况所示
*/
#define FILE_NAME_LENGTH        (116)	//文件名称包括后缀最大占116个字符
#define FILE_SIZE_LENGTH        (16) 	//文件数据最大为16个字符

#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NAK                     (0x15)  /* negative acknowledge */
#define CA                      (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1                  (0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  (0x61)  /* 'a' == 0x61, abort by user */

#define YMODEM_MAX_ERRORS       (5)

extern unsigned char  freamNum;
extern unsigned char  freamNum_C;

QByteArray head_packet(QByteArray original_data, unsigned int data_size);
QByteArray data_packet(QByteArray original_data, unsigned int data_size);
QByteArray tail_packet(unsigned int data_size);

#endif // YMODEMPACKET_H
