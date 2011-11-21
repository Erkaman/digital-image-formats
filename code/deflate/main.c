#include "main.h"
#include <string.h>
#include <stdlib.h>

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
    DataContainer decompressed, compressed;

    compressed = getCompressedData(in);

    verbosePrint("Read in compressed data, size: %ld\n",compressed.size);
/*    printData(compressed); */

    decompressed = ZLIB_Decompress(compressed);

    freeDataContainer(compressed);

    writeData(decompressed,out);

    freeDataContainer(decompressed);

    /* print compressed data. */
}

DataContainer getCompressedData(FILE * in)
{
    fpos_t begDataPointer;
    long begData;
    long endData;
    DataContainer data;
    long dataSize;

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

    data = allocateDataContainer(dataSize);

    fread(data.data,sizeof(BYTE),data.size,in);

    return data;
}

