#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gif.h"

/* Used by all images that do not have a local color table */
int * globalColorTable;

/* used by the graphics that specifies their own local color tables */
int * localColorTable;

/* use a flag to specify whether the current image has local color table or not,
   else use the global color table */

GIFHeader header;

void printHelp(void);
void loadGIF(char * file);
UNSIGNED readUnsigned(FILE * fp);

void readImageInfo(FILE * in);
void printImageInfo(FILE * out);

void loadHeader(FILE * in);

int main(int argc, char *argv[])
{
    if(argc == 1){
        printf("No file was specified for loading\n");
        printf("Try --help for more information.\n");
        return 1;
    } else
        if(!strcmp("--help",argv[1]))
            printHelp();
    loadGIF(argv[1]);

    return 0;
}

UNSIGNED readUnsigned(FILE * fp)
{
    UNSIGNED s;
    fread(&s,sizeof(UNSIGNED),1,fp);
    return s;
}

void printHelp(void)
{
    printf("Usage: gif IN\n");
    printf("Dump the information and color data of a GIF file.\n");
    printf("  --help\tDisplay this help message.\n");

}

void loadGIF(char * file)
{
    FILE * in;
    FILE * out;

    in = fopen(file,"rb");
    assertFileOpened(in);

    file = changeExtension(file,"dmp");
    out = fopen(file,"wb");
    assertFileOpened(out);
    free(file);

    /* read the image info; things like color table and headers */
    readImageInfo(in);

    fprintf(out,"Image info:\n");

    printImageInfo(out);

    fprintf(out,"Color data:\n");

    fclose(in);
    fclose(out);
}

void readImageInfo(FILE * in)
{
    loadHeader(in);
}

void loadHeader(FILE * in)
{
    readStr(in,3,header.signature);
    header.signature[3] = '\0';

    readStr(in,3,header.version);
    header.version[3] = '\0';
}

void printImageInfo(FILE * out)
{
    fprintf(out,"Signature:%s\n",header.signature);
    fprintf(out,"Version:%s\n",header.version);
}
