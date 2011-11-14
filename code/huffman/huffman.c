#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common.h"

int verbose;

void printHelp(void);

void huffmanCompress(FILE * in,FILE * out);
void huffmanDecompress(FILE * in,FILE * out);

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;
    char * outName;
    char * inName;
    int decompress = 0;
    char extension[5];

    verbose = 0;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else {

        /* do the command line parsing */

        ++argv;
        --argc;
        while(1){

            if(!strcmp("--help",*argv)){
                printHelp();
                return 0;
            }
            else if(!strcmp("-d",*argv))
                decompress = 1;
            else if(!strcmp("-v",*argv))
                verbose = 1;

            ++argv;
            --argc;
        }

        inName = *argv;
        in = fopen(inName,"rb");
        assertFileOpened(in);

        if(decompress){

            strncpyBack(extension,inName,4);

            if(!strcmp(extension,".hfm")){
                outName = changeExtension(inName,"unc");

                out = fopen(outName,"wb");
                free(outName);
                assertFileOpened(out);
            }else{

                outName = strAppend(*argv,".unc");
                out = fopen(outName,"wb");
                free(outName);
                assertFileOpened(out);
            }

            huffmanDecompress(in,out);
        }else{
            outName = strAppend(inName,".lzw");
            out = fopen(outName,"wb");
            free(outName);
            assertFileOpened(out);

            huffmanCompress(in,out);
        }

        fclose(in);
        fclose(out);
    }

    return 0;
}

void printHelp(void)
{
    printf("Usage: huffman IN\n");
    printf("Compress or decompress a Huffman coded file.\n"
           "Huffman Coding is done on a file by default");
    printf("  --help\tDisplay this help message.\n");
    printf("  -d\tPerform decompression.\n");
    printf("  -v\tVerbose output.\n");
}

void huffmanCompress(FILE * in,FILE * out)
{
    in = in;
    out = out;
}

void huffmanDecompress(FILE * in,FILE * out)
{
    in = in;
    out = out;
}
