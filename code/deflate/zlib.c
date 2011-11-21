#include "zlib.h"

void checkCheckBits(BYTE cmfByte,BYTE flgByte,ZLIB_FLG flg)
{
    if(!multipleOf(cmfByte * 256 + flgByte,31)){
        printWarning("cmf * 256 + flg is not a multiple of 31;\n"
                     "FCHECK is invalid and CMF and/or FLG data is possibly corrupt.");
    }

    flg = flg;
}

int multipleOf(int n,int mult)
{
    return (n % mult) == 0;
}

DataContainer ZLIB_Decompress(DataContainer data)
{
    ZLIB_CMF cmf;
    ZLIB_FLG flg;

    BYTE cmfByte;
    BYTE flgByte;

    DataContainer decompressed;

    /* the first byte is the CMF byte */
    cmfByte = data.data[0];
    flgByte = data.data[1];

    cmf = readZLIB_CMF(cmfByte);
    printZLIB_CMF(cmf);

    flg = readZLIB_FLG(flgByte);
    printZLIB_FLG(flg);

    checkCheckBits(cmfByte,flgByte,flg);

    /* read dictionary if necessary. */

    /* decompress. */

    decompressed = deflateDecompress(data);

    /* the decompressor only works to the last block, then we here
     validate the checksum.

     Remember: ONLY THE DECOMPRESSED DATA COUNTS, and no dictionary count in. */

    return decompressed;
}


ZLIB_CMF readZLIB_CMF(BYTE cmfByte)
{
    ZLIB_CMF cmf;
    BYTE b;

    b = cmfByte;

    /* print warning message if CM is undefined */
    cmf.CM = b & (~(~0 << 4));

    if(cmf.CM != CM_DEFLATE){
        printWarning("CMF.CM != 8, defaulting to 8\n");
        cmf.CM = CM_DEFLATE;
    }

    cmf.CINFO = (b & (~0 << 4)) >> 4;
    if(cmf.CINFO > CINFO_MAX){
        printWarning("CMF.CINFO > 7, defaulting to 7\n");
        cmf.CINFO = 7;
    }

    return cmf;
}

void printZLIB_CMF(ZLIB_CMF cmf)
{
    verbosePrint("CMF:\n");
    verbosePrint("CM(Compression Method): %d\n",cmf.CM);
    verbosePrint("CINFO(Compression Info): %d\n",cmf.CINFO);
    verbosePrint("\n");
}

ZLIB_FLG readZLIB_FLG(BYTE flgByte)
{
    ZLIB_FLG flg;
    BYTE b;

    b = flgByte;

    flg.FCHECK = b & (~(~0 << 5));

    flg.FDICT = (b & (1 << 5)) >> 5;

    flg.FLEVEL = (b & (~0 << 6)) >> 6;

    return flg;
}
void printZLIB_FLG(ZLIB_FLG flg)
{
    verbosePrint("FLG:\n");
    verbosePrint("FCHECK(Check bits for CMF and FLG): %d\n",flg.FCHECK);
    verbosePrint("FDICT(Preset Dictionary Flag): %d\n",flg.FDICT);

    verbosePrint("FLEVEL(Compression Level): %d(",flg.FLEVEL);

    switch(flg.FLEVEL){
    case FLEVEL_FASTEST_ALGORITHM:
        verbosePrint("Fastest Algorithm");
        break;
    case FLEVEL_FAST_ALGORITHM:
        verbosePrint("Fast Algorithm");
        break;
    case FLEVEL_DEFAULT_ALGORITHM:
        verbosePrint("Default Algorithm");
        break;
    case FLEVEL_SLOWEST_ALGORITHM:
        verbosePrint("Slowest Algorithm, Maximum Compression");
        break;
    }
    verbosePrint(")\n");
    verbosePrint("\n");
}

#define MOD_ADLER 65521

unsigned long adler32(DataContainer data)
{
    /* also: check first implementation of this algorithm. */
    unsigned long a = 1, b = 0;
    unsigned long index;

    for (index = 0; index < data.size; ++index)
    {
        verbosePrint("addler32:number:%x\n",data.data[index]);
        a = (a + data.data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;

        verbosePrint("addler32:a:%d,b:%d\n",a,b);

    }

    return (b << 16) | a;
}
