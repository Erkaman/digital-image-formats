#include "bitwise.h"

BYTE firstNBits(BYTE num,unsigned int n)
{
    return num &  (~(~0 << n));
}

BYTE lastNBits(BYTE num,unsigned int n, int bits)
{
    return (num & (~0 << (bits - n))) >> (bits - n);
}

int getbits(int b, int start, int end)
{
    int len = end - start + 1;
    return (b >> start) & ~(~0 << (len));
}

/*
BYTE lastNBits(BYTE num,unsigned int n)
{
    return (num & (~0 << (8 - n))) >> (8 - n);
}
*/
