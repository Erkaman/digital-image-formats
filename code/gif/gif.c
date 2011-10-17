#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gif.h"

/* Used by all images that do not have a local color table */
unsigned long * globalColorTable;

/* used by the graphics that specifies their own local color tables */
int * localColorTable;

/* use a flag to specify whether the current image has local color table or not,
   else use the global color table */

GIFHeader header;
GIFLogicalScreenDescriptor logicalScreenDescriptor;

void printHelp(void);
void loadGIF(char * file);

UNSIGNED readUnsigned(FILE * fp);
void readImageInfo(FILE * in);

void printImageInfo(FILE * out);
void printSignature(FILE * out);
void printLogicalScreenDescriptor(FILE * out);
void printGlobalColorTable(FILE * out);
void printTableColor(int index,unsigned long * colorTable,FILE * out);

void loadHeader(FILE * in);
void loadLogicalScreenDescriptor(FILE * in);
void loadGlobalColorTable(FILE * in);


/* the color depth of the colors in a GIF is always 24 */
#define COLOR_DEPTH 24

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

    fprintf(out,"* Image info:\n");

    printImageInfo(out);

    fprintf(out,"* Color data:\n");

    fclose(in);
    printf("hello\n");

    /* causes segfault for some reason; fix later */
    fclose(out);
}

void readImageInfo(FILE * in)
{
    loadHeader(in);
    loadLogicalScreenDescriptor(in);
    if(logicalScreenDescriptor.globalColorTableFlag)
	loadGlobalColorTable(in);
}

void loadHeader(FILE * in)
{
    readStr(in,3,header.signature);
    header.signature[3] = '\0';

    readStr(in,3,header.version);
    header.version[3] = '\0';
}

void loadLogicalScreenDescriptor(FILE * in)
{
    BYTE packedFields;

    logicalScreenDescriptor.logicalScreenWidth = readUnsigned(in);
    logicalScreenDescriptor.logicalScreenHeight = readUnsigned(in);

    packedFields = readByte(in);
    logicalScreenDescriptor.globalColorTableFlag = (packedFields & (1 << 7)) >> 7;
    logicalScreenDescriptor.colorResolution = (packedFields & (7 << 4)) >> 4;
    logicalScreenDescriptor.sortFlag = (packedFields & (1 << 3)) >> 3;
    logicalScreenDescriptor.globalColorTableSize = (packedFields & 7);

    logicalScreenDescriptor.backgroundColorIndex = readByte(in);
    logicalScreenDescriptor.pixelAspectRatio = readByte(in);
}

void printImageInfo(FILE * out)
{
    printSignature(out);
    printLogicalScreenDescriptor(out);
    if(logicalScreenDescriptor.globalColorTableFlag)
	printGlobalColorTable(out);
}

void printSignature(FILE * out)
{
    fprintf(out,"** Header:\n");
    fprintf(out,"Signature:%s\n",header.signature);
    fprintf(out,"Version:%s\n",header.version);
}

void printLogicalScreenDescriptor(FILE * out)
{
    fprintf(out,"** Logical Screen Descriptor:\n");

    fprintf(out,"Logical Screen Width:%d\n",
	    logicalScreenDescriptor.logicalScreenWidth);
    fprintf(out,"Logical Screen Height:%d\n",
	    logicalScreenDescriptor.logicalScreenHeight);

    fprintf(out,"Global Color Table Flag:%d\n",
	    logicalScreenDescriptor.globalColorTableFlag);
    fprintf(out,"Color Resolution:%d\n",
	    logicalScreenDescriptor.colorResolution);
    fprintf(out,"Sort Flag:%d\n",
	    logicalScreenDescriptor.sortFlag);
    fprintf(out,"Global Color Table Size:%d\n",
	    logicalScreenDescriptor.globalColorTableSize);

    fprintf(out,"Background Color Index:%d\n",
	    logicalScreenDescriptor.backgroundColorIndex);
    fprintf(out,"Pixel Aspect Ratio:%d\n",
	    logicalScreenDescriptor.pixelAspectRatio);
}

void loadGlobalColorTable(FILE * in)
{
    unsigned long color;
    int realGlobalColorTableSize;
    int i;

    realGlobalColorTableSize = pow(2,1 + logicalScreenDescriptor.globalColorTableSize);

    globalColorTable = (unsigned long *) malloc(realGlobalColorTableSize);

    for(i = 0; i <  realGlobalColorTableSize; ++i){
        color  = 0;
        fread(&color, COLOR_DEPTH / 8, 1, in);
        globalColorTable[i] = color;
    }
}

void printGlobalColorTable(FILE * out)
{
    int realGlobalColorTableSize;
    int i;

    realGlobalColorTableSize = pow(2,1 + logicalScreenDescriptor.globalColorTableSize);

    for(i = 0; i <  realGlobalColorTableSize; ++i)
        printTableColor(i,globalColorTable,out);
}

void printTableColor(int index,unsigned long * colorTable,FILE * out)
{
    unsigned long r,g,b;
    unsigned long color;

    color = colorTable[index];

    /* because the color is stored least significant bit first. */
    b = (color & (0xff << 16)) >> 16;
    g = (color & (0xff << 8)) >> 8;
    r = color & 0xff;

    out = out;
    fprintf(out,"%d:(%lu,%lu,%lu)\n",index,r,g,b);
}
