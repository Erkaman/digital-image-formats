#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common.h"

void printHelp(void);
void lzw_compress(FILE * in,FILE * our);

int main(int argc, char *argv[])
{
    int decompress = 0;
    FILE * in;
    FILE * out;
    char * outFile;
    char * inFile;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else{

	++argv;

        if(!strcmp("--help",*argv)){
            printHelp();
            return 0;
        }
        if(!strcmp("-d",*argv)){
            decompress = 1;
            ++argv;
        }


        if(decompress){

        }else{
	    inFile = *argv;
            in = fopen(inFile,"rb");
            assertFileOpened(in);

	    outFile = (char *)malloc(strlen(*argv) + 4);
	    strcpy(outFile,*argv);
	    strcat(outFile,".lzw");
            out = fopen(outFile,"wb");
	    free(outFile);
            assertFileOpened(out);

	    lzw_compress(in,out);
	    fclose(in);
	    fclose(out);
        }

        /* load*/
    }

    return 0;
}

void printHelp(void)
{
    printf("Usage: lzw IN\n");
    printf("Compress or decompress a LZW encoded file.\n"
           "Compression is done by default");
    printf("  --help\tDisplay this help message.\n");
    printf("  -d\tPerform decompression.\n");

}

void lzw_compress(FILE * in,FILE * out)
{
    in = in;
    out = out;
}
