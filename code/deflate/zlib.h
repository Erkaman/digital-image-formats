#ifndef _ZLIB_H_
#define _ZLIB_H_

#include "../common.h"
#include "deflate.h"

typedef struct{
    BYTE CM; /* Compression Method */
    BYTE CINFO; /* Compression Info */
    /* LZ77 window size is calculated according to 2^(CINFO + 8)*/
} ZLIB_CMF;

#define CM_DEFLATE 8
#define CM_RESERVED 15

#define CINFO_MAX 7

typedef struct{
    BYTE FCHECK; /* check bits for CMF and FLG */
    BYTE FDICT;
    BYTE FLEVEL;
} ZLIB_FLG;

#define FLEVEL_FASTEST_ALGORITHM 0
#define FLEVEL_FAST_ALGORITHM 1
#define FLEVEL_DEFAULT_ALGORITHM 2
#define FLEVEL_SLOWEST_ALGORITHM 3

DataContainer ZLIB_Decompress(DataContainer data);

int multipleOf(int n,int mult);

void checkCheckBits(BYTE cmfByte,BYTE flgByte,ZLIB_FLG flg);

ZLIB_CMF readZLIB_CMF(BYTE cmfByte);
void printZLIB_CMF(ZLIB_CMF cmf);

ZLIB_FLG readZLIB_FLG(BYTE flgByte);
void printZLIB_FLG(ZLIB_FLG flg);

unsigned long adler32(DataContainer data);

void validateCheckSum(DataContainer data, DataContainer decompressed);


#endif /* _ZLIB_H_ */
