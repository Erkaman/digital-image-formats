#include "zlib.h"
#include <cmath>
#include <cmath>
#include <cstdlib>
#include "deflate.h"
#include "../io.h"

#include "../defs.h"

using std::vector;

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


int multipleOf(int n,int mult);

void checkCheckBits(BYTE cmfByte,BYTE flgByte,ZLIB_FLG flg);

ZLIB_CMF readZLIB_CMF(BYTE cmfByte);
void printZLIB_CMF(ZLIB_CMF cmf);

ZLIB_FLG readZLIB_FLG(BYTE flgByte);
void printZLIB_FLG(ZLIB_FLG flg);

unsigned long adler32(vector<BYTE> data);

void validateCheckSum(vector<BYTE> data, vector<BYTE> decompressed);


void checkCheckBits(BYTE cmfByte,BYTE flgByte,ZLIB_FLG flg)
{
    if(!multipleOf(cmfByte * 256 + flgByte,31)){
        printWarning("cmf * 256 + flg is not a multiple of 31;\n"
                     "FCHECK is invalid and CMF and/or FLG data is possibly corrupt.");
    } else {
	verbosePrint("FLG checkbits are correct.\n");
    }

    flg = flg;
}

int multipleOf(int n,int mult)
{
    return (n % mult) == 0;
}

vector<BYTE> ZLIB_Decompress(vector<BYTE> data)
{
    ZLIB_CMF cmf;
    ZLIB_FLG flg;

    BYTE cmfByte;
    BYTE flgByte;

    vector<BYTE> decompressed;

    /* the first byte is the CMF byte */
    cmfByte = data[0];
    flgByte = data[1];

    cmf = readZLIB_CMF(cmfByte);
    printZLIB_CMF(cmf);

    flg = readZLIB_FLG(flgByte);
    printZLIB_FLG(flg);

    checkCheckBits(cmfByte,flgByte,flg);

    /* read dictionary if necessary. */

    /* decompress. */
    decompressed = deflateDecompress(data);

    validateCheckSum(data,decompressed);

    /* the decompressor only works to the last block, then we here
     validate the checksum.

     Remember: ONLY THE DECOMPRESSED DATA COUNTS, and no dictionary count in. */

    return decompressed;
}

void validateCheckSum(vector<BYTE> data, vector<BYTE> decompressed)
{
    /* Does this work for computers of different Endian? */
    unsigned long calcChecksum;
    unsigned long dataChecksum;

    calcChecksum = adler32(decompressed);

    dataChecksum =
	data[data.size() - 4] * pow(256,3) +
	data[data.size() - 3] * pow(256,2) +
        data[data.size() - 2] * pow(256,1) +
	data[data.size() - 1] * pow(256,0);

    verbosePrint("checkSums: calculated = %ld. Proper = %ld\n",calcChecksum,dataChecksum);

    if(calcChecksum != dataChecksum){
	printError("ZLIB decompressed data check does not match, corrupted data!");
	exit(1);
    } else {

	verbosePrint("Correct ZLIB checksum!\n");

    }
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

unsigned long adler32(vector<BYTE> data)
{
    /* also: check first implementation of this algorithm. */
    unsigned long a = 1, b = 0;
    unsigned long index;

    for (index = 0; index < data.size(); ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }

    return (b << 16) | a;
}

