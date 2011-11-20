#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "main.h"
#include "../common.h"
#include "deflate.h"

/* 1644192256 */

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;
    char * outName;
    char * inName;
    int decompress = 0;
    char extension[6];
/*    DataContainer data; */


    verbose = 1;

/*    data = allocateDataContainer(6);

      i = 0;


      data.data[i++] = 0x01;
      data.data[i++] = 0x00;
      data.data[i++] = 0x01;
      data.data[i++] = 0xff;
      data.data[i++] = 0xfe;
      data.data[i++] = 0x61;
      verbosePrint("%ld\n",adler32(data));*/

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
        return 0;
    }

    /* do the command line parsing */

    ++argv;
    while(1){

        if(!strcmp("--help",*argv)){
            printHelp();
            return 0;
        }
        else if(!strcmp("-d",*argv))
            decompress = 1;
        else if(!strcmp("-v",*argv))
            verbose = 1;
        else
            break;

        ++argv;
    }


    inName = *argv;
    in = fopen(inName,"rb");
    assertFileOpened(in);

    if(decompress){

        strncpyBack(extension,inName,5);

        if(!strcmp(extension,".zlib")){
            outName = changeExtension(inName,".unc");
            out = fopen(outName,"wb");
            assertFileOpened(out);
        }else{

            /* TODO: code doesn't work for file names like test.com */
            outName = strAppend(*argv,".unc");
            out = fopen(outName,"wb");
            assertFileOpened(out);
        }

        verbosePrint("%s\n",inName);
        verbosePrint("%s\n",outName);

        ZLIB_Decompress(in,out);

        free(outName);

    }else{
        outName = strAppend(inName,".zlib");
        out = fopen(outName,"wb");
        assertFileOpened(out);

/*            huffmanCompress(in,out); */

        free(outName);
    }

    fclose(in);
    fclose(out);

    return 0;
}

void printHelp(void)
{
    /* TODO: fix*/
    printf("Usage: deflate IN\n");
    printf("Compress or decompress a DEFLATE compressed file.\n"
           "Huffman Coding is done on a file by default");
    printf("  --help\tDisplay this help message.\n");
    printf("  -d\tPerform decompression.\n");
    printf("  -v\tVerbose output.\n");
}

void ZLIB_Decompress(FILE * in, FILE * out)
{
    ZLIB_CMF cmf;
    ZLIB_FLG flg;

    BYTE cmfByte;
    BYTE flgByte;

    DataContainer data;

    DataContainer decompressed;

    unsigned int i;

    cmfByte = readByte(in);
    flgByte = readByte(in);

    cmf = readZLIB_CMF(cmfByte);
    printZLIB_CMF(cmf);

    flg = readZLIB_FLG(flgByte);
    printZLIB_FLG(flg);

    checkCheckBits(cmfByte,flgByte,flg);

    /* read dictionary of necessary. */

    data = getCompressedData(in);

    /* decompress. */

/*    verbosePrint("next\n");
      for(i = 0; i < 6; ++i){
      verbosePrint("%x\n",data.data[i]);
      }*/

    decompressed = deflateDecompress(data);


    verbosePrint("decompressed\n");
    for(i = 0; i < decompressed.size; ++i){
        verbosePrint("%d\n",decompressed.data[i]);
    }

    freeDataContainer(data);

    verbosePrint("adler32:%d",adler32(decompressed));


    out = out;
}

DataContainer getCompressedData(FILE * in)
{
    fpos_t begDataPointer;
    long begData;
    long endData;
    DataContainer data;
    long dataSize;
/*    unsigned long checkSum; */
    begData = ftell(in);

    /* Save the start position of the data. */
    if(fgetpos(in, &begDataPointer) != 0){
        printError("Couldn't save beginning of data pointer in getCompressedData().\n");
        exit(1);
    }

    fseek(in,-4,SEEK_END);
    endData = ftell(in);

    fsetpos(in,&begDataPointer);

    dataSize = endData - begData;

    verbosePrint("data size: %ld\n",dataSize);

    data = allocateDataContainer(dataSize);


    fread(data.data,sizeof(BYTE),data.size,in);

    /* check checksum.
       fread(&checkSum,4,1,in);
       verbosePrint("checkSum:%ld\n",checkSum);

       verbosePrint("aldr32:%ld\n",adler32(data));*/

    return data;
/*    fread() */

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

void printWarning(const char * format, ...)
{
    va_list vl;

    printf("Warning: ");

    va_start(vl, format);
    vprintf(format, vl);
    va_end(vl);
}

void printError(const char * format, ...)
{
    va_list vl;

    printf("ERROR: ");

    va_start(vl, format);
    vprintf(format, vl);
    va_end(vl);
}
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

#define MOD_ADLER 65521

unsigned long adler32(DataContainer data)
{
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
/*
  #define BASE 65521
  #define NMAX 5552

  #define DO1(buf)  {s1 += *buf++; s2 += s1;}
  #define DO2(buf)  DO1(buf); DO1(buf);
  #define DO4(buf)  DO2(buf); DO2(buf);
  #define DO8(buf)  DO4(buf); DO4(buf);
  #define DO16(buf) DO8(buf); DO8(buf);

  unsigned long adler32Real(unsigned long adler, BYTE * buf, unsigned long len)
  {
  unsigned long s1 = adler & 0xffff;
  unsigned long s2 = (adler >> 16) & 0xffff;
  int k;

  while (len > 0) {
  k = len < NMAX ? len : NMAX;
  len -= k;
  while (k >= 16) {
  DO16(buf);
  k -= 16;
  }
  if (k != 0) do {
  DO1(buf);
  } while (--k);
  s1 %= BASE;
  s2 %= BASE;
  }
  return (s2 << 16) | s1;
  }
*/
