#ifndef _MAIN_H_
#define _MAIN_H_

#include "../common.h"

typedef struct{
    BYTE CM; /* Compression Method */
    BYTE CINFO; /* Compression Info */
    /* LZ77 window size is calculated according to 2^(CINFO + 8)*/
} ZLIB_CMF;

typedef struct{
    BYTE FCHECK; /* check bits for CMF and FLG */
    BYTE FDICT;
    BYTE FLEVEL;
} ZLIB_FLG;

void printHelp(void);

void ZLIB_Decompress(FILE * in, FILE * out);

ZLIB_CMF readZLIB_CMF(BYTE cmfByte);
void printZLIB_CMF(ZLIB_CMF cmf);

ZLIB_FLG readZLIB_FLG(BYTE flgByte);
void printZLIB_FLG(ZLIB_FLG flg);

void checkCheckBits(BYTE cmfByte,BYTE flgByte,ZLIB_FLG flg);

void printWarning(const char * format, ...);

int multipleOf(int n,int mult);

#endif /* _MAIN_H_ */
