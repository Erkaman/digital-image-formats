#include "main.h"
#include "png.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;

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

    dumpPNG(in,out);

    fclose(in);
    fclose(out);

    return 0;
}

void printHelp(void)
{
    printf("Usage: deflate IN OUT\n");
    printf("Dump the contents of a PNG file. .\n");
    printf("  --help\tDisplay this help message.\n");
    printf("  -v\tVerbose output.\n");
}