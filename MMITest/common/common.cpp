#include "common.h"

float bytefloat(unsigned char a)
BEIGN
    unsigned int l;
    float f;

    l = a;
    f = *((float *)(&l));

    return f;
END

float byte1float(unsigned char a, unsigned char b)
BEIGN

    unsigned int l;
    float f;

    l = a;
    l &= 0xff;
    l |= ((long) b << 8);
    f = *((float *)(&l));

    return f;
END

float byte2float(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
BEIGN

    unsigned int l;
    float f;

    l = a;
    l &= 0xff;
    l |= ((long) b << 8);
    l &= 0xffff;
    l |= ((long) c << 16);
    l &= 0xffffff;
    l |= ((long) d << 24);
    f = *((float *)(&l));

    return f;
END

long byte1int(unsigned char  a, unsigned char  b)
BEIGN
    long l;

    l = a;
    l &= 0xff;
    l |= ((long) b << 8);

    return l;
END

long byte2int(unsigned char  a, unsigned char  b, unsigned char  c, unsigned char  d)
BEIGN
    long l;

    l = a;
    l &= 0xff;
    l |= ((long) b << 8);
    l &= 0xffff;
    l |= ((long) c << 16);
    l &= 0xffffff;
    l |= ((long) d << 24);

    return l;
END




