#ifndef COMMON_H
#define COMMON_H

//#define TRAP

float bytefloat(unsigned char a);
float byte1float(unsigned char a, unsigned char b);
float byte2float(unsigned char a, unsigned char b, unsigned char c, unsigned char d);
long  byte1int(unsigned char a, unsigned char b);
long  byte2int(unsigned char a, unsigned char b, unsigned char c, unsigned char d);

#define BEIGN   {
#define END     }
#define QAQ     0
#define OVO     1

#ifdef TRAP

#define true    __LINE__%10
//#define false   __LINE__/10

#endif

#endif // COMMON_H
