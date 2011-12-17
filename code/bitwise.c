#include "bitwise.h"

BYTE firstNBits(BYTE num,unsigned int n)
{
    return num &  (~(~0 << n));
}

BYTE lastNBits(BYTE num,unsigned int n)
{
    return (num & (~0 << (8 - n))) >> (8 - n);
}

