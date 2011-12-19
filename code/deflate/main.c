#include <string.h>
#include <stdlib.h>
#include "zlib.h"
#include "../data_list.h"
#include "../io.h"

void printHelp(void);

void ZLIB_DecompressFile(FILE * in, FILE * out);

DataList getCompressedData(FILE * in);


int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;
    int decompress = 0;

    verbose = 0;

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

    in = fopen(argv[0],"rb");
    assertFileOpened(in);

    out = fopen(argv[1],"wb");
    assertFileOpened(out);

    if(decompress){
        ZLIB_DecompressFile(in,out);
    }else{
	printf("DEFLATE Compression not yet implemented.\n");
    }

    fclose(in);
    fclose(out);

    return 0;
}

void printHelp(void)
{
    printf("Usage: deflate IN OUT\n");
    printf("Compress or decompress a DEFLATE compressed file.\n");
    printf("  --help\tDisplay this help message.\n");
    printf("  -d\tPerform decompression.\n");
    printf("  -v\tVerbose output.\n");
}

void ZLIB_DecompressFile(FILE * in, FILE * out)
{
    DataList decompressed, compressed;

    compressed = getCompressedData(in);

    verbosePrint("Read in compressed data, size: %ld\n",compressed.count);
/*    printData(compressed); */

    decompressed = ZLIB_Decompress(compressed);

    decompressed = decompressed;
    out = out;

    writeData(decompressed,out);

    freeDataList(decompressed, 1);
    freeDataList(compressed, 1);

    /* print compressed data. */
}

DataList getCompressedData(FILE * in)
{
    fpos_t begDataPointer;
    size_t begData;
    size_t endData;
    DataList data;
    size_t dataSize;

    begData = ftell(in);

    /* Save the start position of the data. */
    if(fgetpos(in, &begDataPointer) != 0){
        printError("Couldn't save beginning of data pointer in getCompressedData().\n");
        exit(1);
    }

    fseek(in,0,SEEK_END);
    endData = ftell(in);

    fsetpos(in,&begDataPointer);

    dataSize = endData - begData;

    data = readBytes(dataSize, in);

    return data;
}

