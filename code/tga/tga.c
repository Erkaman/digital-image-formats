#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tga.h"
#include "../common.h"

/* getbits:  get n bits from position p */
static unsigned getbits(unsigned x, int p, int n);

static SHORT readShort(FILE * fp);

static void readStr(FILE * fp,size_t length,char * str);

static void getImageDestStr(char * str,int imageOrigin);

extern void loadTGAHeader(TGAHeader * tgah,FILE * fp);

int main(int argc, char * argv[])
{
    if(argc == 1){
        printf("No file was specified for loading");
        return 1;
    } else{
        loadTGA(argv[1]);
    }
    return 0;
}


void printRGB(unsigned long r,unsigned long g,unsigned long b,FILE * fp)
{
    fprintf(fp,"(%lu , %lu , %lu)",r,g,b);
}

void printGrayScaleRGB(unsigned long d,FILE * fp)
{
    printRGB(d,d,d,fp);
}

void loadTGA(char * file)
{
    BYTE alphaChannelBits;
    BYTE imageDest;
    char imageDestStr[11];
    FILE * in;
    FILE * out;
    char imageID[255];
    int col,row;
    unsigned long data;
/*    SHORT * colorMap; */
/*    int i ; */
    TGAHeader tgah;

    in = fopen(file,"rb");

    if(in == NULL){
        perror("An error occurred opening the file");
        exit(1);
    }

    loadTGAHeader(&tgah,in);

    if(tgah.IDLength > 0)
        readStr(in,tgah.IDLength * sizeof(char),imageID);

    file = changeExtension(file,"dmp");

    out = fopen(file,"wb");
    free(file);

    alphaChannelBits = getbits(tgah.imageDescriptor,3,4);
    imageDest = getbits(tgah.imageDescriptor,5,2);
    getImageDestStr(imageDestStr,imageDest);

/*    if(tgah.colorMapType != 0){
        if(tgah.colorMapDepth == 16){
            colorMap = malloc(sizeof(SHORT) * tgah.colorMapLength);
            fread(colorMap,sizeof(SHORT),tgah.colorMapLength,in);
        }
    }*/

    fprintf(out,"Id Length:%d\n",tgah.IDLength);
    fprintf(out,"Color map type:%d\n",tgah.colorMapType);
    fprintf(out,"Image type:%d\n",tgah.imageType);
    fprintf(out,"Color map start:%d\n",tgah.colorMapStart);
    fprintf(out,"Color map length:%d\n",tgah.colorMapLength);
    fprintf(out,"Color map depth:%d\n",tgah.colorMapDepth);
    fprintf(out,"X origin:%d\n",tgah.xOrigin);
    fprintf(out,"Y origin:%d\n",tgah.yOrigin);
    fprintf(out,"Width:%d\n",tgah.width);
    fprintf(out,"Height:%d\n",tgah.height);
    fprintf(out,"pixelDepth:%d\n",tgah.pixelDepth);
    fprintf(out,"Image Descriptor:%d\n",tgah.imageDescriptor);
    fprintf(out,"Alpha Channel Bits:%d\n",alphaChannelBits);
    fprintf(out,"Screen destination of first pixel:%s\n",imageDestStr);
    fprintf(out,"Image ID:%s\n",imageID);

    /* read color map */
/*    fprintf(out,"Color map:");
    if(tgah.colorMapDepth == 0)
        fprintf(out,"None\n");
    else{
        fprintf(out,"\n");
        for(i = 0; i < tgah.colorMapLength; ++i)
            fprintf(out,"%d\n",colorMap[i]);
    }*/

    /* read color data */
    if(tgah.imageType == 3 && tgah.colorMapType == 0){
	for(row = 0; row < tgah.height; ++row){
	    for(col = 0; col < tgah.width; ++col){
		fread(&data, tgah.pixelDepth / 8, 1, in);
		printGrayScaleRGB(data,out);
	    }

	}
	fprintf(out,"\n");
    }

    /*    free(colorMap);*/

    fclose(in);
}

static unsigned getbits(unsigned x, int p, int n)
{
    return (x >> (p+1-n)) & ~(~0 << n);
}

static SHORT readShort(FILE * fp)
{
    SHORT s;
    fread(&s,sizeof(SHORT),1,fp);
    return s;
}

static void readStr(FILE * fp,size_t length,char * str)
{
    fread(str,sizeof(CHAR),length,fp);
}

static void getImageDestStr(char * str,int imageOrigin)
{
    switch(imageOrigin){
    case 0:
        strcpy(str,"Bottom Left");
        break;
    case 1:
        strcpy(str,"Bottom Rigth");
        break;
    case 2:
        strcpy(str,"Top Left");
        break;
    case 3:
        strcpy(str,"Top Right");
        break;
    }
}

extern void loadTGAHeader(TGAHeader * tgah,FILE * fp)
{
    tgah->IDLength = readByte(fp);
    tgah->colorMapType = readByte(fp);
    tgah->imageType = readByte(fp);
    tgah->colorMapStart = readShort(fp);
    tgah->colorMapLength = readShort(fp);
    tgah->colorMapDepth = readByte(fp);
    tgah->xOrigin = readShort(fp);
    tgah->yOrigin = readShort(fp);
    tgah->width = readShort(fp);
    tgah->height = readShort(fp);
    tgah->pixelDepth = readByte(fp);
    tgah->imageDescriptor = readByte(fp);
}
