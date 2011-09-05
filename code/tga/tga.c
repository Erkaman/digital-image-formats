#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tga.h"
#include "../common.h"

/* getbits:  get n bits from position p */
static unsigned getbits(unsigned x, int p, int n);

static SHORT readShort(FILE * fp);
static LONG readLong(FILE * fp);
static char readChar(FILE * fp);
static void readStr(FILE * fp,size_t length,char * str);

static void getImageDestStr(char * str,int imageOrigin);

extern void loadTGAHeader(TGAHeader * tgah,FILE * fp);
extern int loadTGAExtensionArea(TGAExtensionArea * tgaex,FILE * fp);

void printFormatAuthorComment(char * authorComment,FILE * fp);

void printRGB(unsigned long r,unsigned long g,unsigned long b,FILE * fp);
void printGrayScaleRGB(unsigned long d,FILE * fp);

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
    int hasExtensionArea;
    /* the author comment field organized into lines. */
/*    SHORT * colorMap; */
/*    int i ; */
    TGAHeader tgah;
    TGAExtensionArea tgaex;

    in = fopen(file,"rb");

    if(in == NULL){
        perror("An error occurred opening the file");
        exit(1);
    }

    hasExtensionArea = loadTGAExtensionArea(&tgaex,in);
    rewind(in);
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

    if(hasExtensionArea){
        fprintf(out,"Version:%s\n","2.0");
        fprintf(out,"Extension Area:\n");
        fprintf(out,"Extension Size:%d\n",tgaex.size);
        fprintf(out,"Author Name:%s\n",tgaex.authorName);

        fprintf(out,"Author Comment:\n");
        printFormatAuthorComment(tgaex.authorComment,out);
        fprintf(out,"End Author Comment:\n");

        fprintf(out,"Stamp date/time: %d/%d - %d %d:%d:%d\n",
                tgaex.stampDay,
                tgaex.stampMonth,
                tgaex.stampYear,
                tgaex.stampHour,
                tgaex.stampMinute,
                tgaex.stampSecond);

        fprintf(out,"Job name: %s\n",tgaex.jobName);

        fprintf(out,"Job time: %d:%d:%d\n",
                tgaex.jobHour,
                tgaex.jobMinute,
                tgaex.jobSecond);

        fprintf(out,"Software ID: %s",tgaex.softwareId);

	if(tgaex.versionNumber != 0 ){
	    fprintf(out," %.2f%c\n",tgaex.versionNumber / 100.0,tgaex.versionLetter);
	}else{
	    fprintf(out,"\n");
	}

        fprintf(out,"Key color: %d\n",tgaex.keyColor);

        fprintf(out,"Pixel Ratio Numerator: %d\n",tgaex.pixelRatioNumerator);
        fprintf(out,"Pixel Ratio Denominator: %d\n",tgaex.pixelRatioDenominator);

        fprintf(out,"End of extension area\n");
    }else
        fprintf(out,"Version:%s\n","1.0");


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

static LONG readLong(FILE * fp)
{
    LONG s;
    fread(&s,sizeof(LONG),1,fp);
    return s;
}


static char readChar(FILE * fp)
{
    char s;
    fread(&s,sizeof(char),1,fp);
    return s;
}

static void readStr(FILE * fp,size_t length,char * str)
{
    fread(str,sizeof(char),length,fp);
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

extern int loadTGAExtensionArea(TGAExtensionArea * tgaex,FILE * fp)
{
    char signature[18];
    LONG extensionAreaOffset;
    /* check the footer. */
    fseek(fp, -26, SEEK_END);
    fseek(fp, 8, SEEK_CUR);

    readStr(fp,18,signature);

    /* it's not the proper signature, so there's no extension area. */
    if(strcmp(signature,"TRUEVISION-XFILE."))
        return 0;

    fseek(fp, -26, SEEK_END);

    /*   printf("%s\n",signature); */

    /* read extension area offset. */

    fread(&extensionAreaOffset,sizeof(LONG),1,fp);

    if(extensionAreaOffset == 0){
        return 0;
    }

    fseek(fp,extensionAreaOffset,SEEK_SET);

    /* read extension area */

    tgaex->size = readShort(fp);
    readStr(fp,41,tgaex->authorName);
    readStr(fp,324,tgaex->authorComment);

    /* read date/time stamp */

    tgaex->stampMonth = readShort(fp);
    tgaex->stampDay = readShort(fp);
    tgaex->stampYear = readShort(fp);
    tgaex->stampHour = readShort(fp);
    tgaex->stampMinute = readShort(fp);
    tgaex->stampSecond = readShort(fp);

    /* read job information */
    readStr(fp,41,tgaex->jobName);
    tgaex->jobHour = readShort(fp);
    tgaex->jobMinute = readShort(fp);
    tgaex->jobSecond = readShort(fp);

    readStr(fp,41,tgaex->softwareId);
    tgaex->versionNumber = readShort(fp);
    tgaex->versionLetter = readChar(fp);

    tgaex->keyColor = readLong(fp);

    tgaex->pixelRatioNumerator = readShort(fp);
    tgaex->pixelRatioDenominator = readShort(fp);

    return 1;
}

void printRGB(unsigned long r,unsigned long g,unsigned long b,FILE * fp)
{
    fprintf(fp,"(%lu , %lu , %lu)",r,g,b);
}
/* C-c @ C-M-h  */
void printGrayScaleRGB(unsigned long d,FILE * fp)
{
    printRGB(d,d,d,fp);
}

void printFormatAuthorComment(char * authorComment,FILE * fp)
{
    int i;

    for(i = 0; i < 4; ++i){
        fprintf(fp,"%d: %s\n",i+1,authorComment);
        authorComment += 81;
    }
}
