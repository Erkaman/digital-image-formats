#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "main.h"
#include "../common.h"

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;
    char * outName;
    char * inName;
    int decompress = 0;
    char extension[6];

    verbose = 1;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else {

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
    }

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

    cmfByte = readByte(in);
    flgByte = readByte(in);

    cmf = readZLIB_CMF(cmfByte);
    printZLIB_CMF(cmf);

    flg = readZLIB_FLG(flgByte);
    printZLIB_FLG(flg);

    checkCheckBits(cmfByte,flgByte,flg);

    out = out;
}

ZLIB_CMF readZLIB_CMF(BYTE cmfByte)
{
    ZLIB_CMF cmf;
    BYTE b;

    b = cmfByte;

    /* print warning message if CM is undefined */
    cmf.CM = b & (~(~0 << 4));

    if(cmf.CM != 8){
	printWarning("CMF.CM != 8, defaulting to 8\n");
	cmf.CM = 8;
    }

    cmf.CINFO = (b & (~0 << 4)) >> 4;
    if(cmf.CINFO > 7){
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
    verbosePrint("FLEVEL(Compression Level): %d\n",flg.FLEVEL);
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
