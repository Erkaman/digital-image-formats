#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tga.h"
#include "../common.h"
#include "util.h"
/* TODO: properly free the color map */
void loadTGAHeader(FILE * fp);
int loadTGAExtensionArea(FILE * fp);
void loadTGA(char * file);

void printFormatAuthorComment(char * authorComment,FILE * fp);
void printRGB(unsigned long r,unsigned long g,unsigned long b,FILE * fp);
void printGrayScaleRGB(unsigned long d,FILE * fp);
void printRGBA(unsigned long r,unsigned long g,unsigned long b,unsigned long a,FILE * fp);
void printColorData(unsigned long data,FILE * out);
void printcompressedImage(SHORT width,SHORT height,FILE * in,FILE * out);
void printImage(SHORT width,SHORT height,FILE * in,FILE * out);

void getImageDestStr(char * str,int imageOrigin);
void readStamp(LONG offset,FILE * in,FILE * out);
void readColorMap(FILE * in,FILE * out);

TGAHeader tgah;
TGAExtensionArea tgaex;
unsigned long * colorMap;

int main(int argc, char * argv[])
{
    if(argc == 1){
        printf("No file was specified for loading\n");
        return 1;
    } else
        loadTGA(argv[1]);

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
    int hasExtensionArea;

    int compressed;

    in = fopen(file,"rb");

    assertFileOpened(in);

    hasExtensionArea = loadTGAExtensionArea(in);
    rewind(in);
    loadTGAHeader(in);

    if(tgah.IDLength > 0)
        readStr(in,(tgah.IDLength) * sizeof(char),imageID);
    imageID[tgah.IDLength] = '\0';

    file = changeExtension(file,"dmp");

    out = fopen(file,"wb");
    assertFileOpened(out);

    free(file);

    alphaChannelBits = getbits(tgah.imageDescriptor,3,4);
    imageDest = getbits(tgah.imageDescriptor,5,2);
    getImageDestStr(imageDestStr,imageDest);

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

        fprintf(out,"Gamma value:");
        if(tgaex.gammaDenominator == 0){
            fprintf(out,"Unused\n");
        } else{
            fprintf(out,"%f\n",(float)tgaex.gammaNumerator / (float)tgaex.gammaDenominator);
        }

        fprintf(out,"color correction offset: %d\n",tgaex.colorOffset);
        fprintf(out,"Postage stamp offset: %d\n",tgaex.stampOffset);
        fprintf(out,"Scan line offset: %d\n",tgaex.scanOffset);

        fprintf(out,"Attributes Type: %d\n",tgaex.attributesType);

        fprintf(out,"End of extension area\n");

    }else
        fprintf(out,"Version:%s\n","1.0");

    /* read the color map */
    if(tgah.colorMapType == COLOR_MAPPED){
        fprintf(out,"Color map:\n");
        readColorMap(in,out);
    }



    fprintf(out,"Color data:\n");


    compressed =
        tgah.imageType == RUN_LENGTH_ENCODED_BLACK_AND_WHITE ||
        tgah.imageType == RUN_LENGTH_ENCODED_TRUE_COLOR ||
        tgah.imageType == RUN_LENGTH_ENCODED_COLOR_MAPPED;


    /* read color data */
    if(!compressed)
        printImage(tgah.width,
                   tgah.height,
                   in,
                   out);
    else
        printcompressedImage(tgah.width,
                             tgah.height,
                             in,
                             out);

    if(tgaex.stampOffset != 0){
        fprintf(out,"Stamp postage:\n");
/*        readStamp(tgaex.stampOffset,in,out); */
    }

    free(colorMap);


    fclose(in);
}


void getImageDestStr(char * str,int imageOrigin)
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

extern void loadTGAHeader(FILE * fp)
{
    tgah.IDLength = readByte(fp);
    tgah.colorMapType = readByte(fp);
    tgah.imageType = readByte(fp);
    tgah.colorMapStart = readShort(fp);
    tgah.colorMapLength = readShort(fp);
    tgah.colorMapDepth = readByte(fp);
    tgah.xOrigin = readShort(fp);
    tgah.yOrigin = readShort(fp);
    tgah.width = readShort(fp);
    tgah.height = readShort(fp);
    tgah.pixelDepth = readByte(fp);
    tgah.imageDescriptor = readByte(fp);
}

extern int loadTGAExtensionArea(FILE * fp)
{
    char signature[18];
    LONG extensionAreaOffset;
    /* check the footer. */
    fseek(fp, -26, SEEK_END);
    fseek(fp, 8, SEEK_CUR);

    readStr(fp,18,signature);

    /* read job information */
    /* It's not the proper signature, so there's no extension area. */
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

    tgaex.size = readShort(fp);
    readStr(fp,41,tgaex.authorName);
    readStr(fp,324,tgaex.authorComment);

    /* read date/time stamp */

    tgaex.stampMonth = readShort(fp);
    tgaex.stampDay = readShort(fp);
    tgaex.stampYear = readShort(fp);
    tgaex.stampHour = readShort(fp);
    tgaex.stampMinute = readShort(fp);
    tgaex.stampSecond = readShort(fp);

    readStr(fp,41,tgaex.jobName);
    tgaex.jobHour = readShort(fp);
    tgaex.jobMinute = readShort(fp);
    tgaex.jobSecond = readShort(fp);

    readStr(fp,41,tgaex.softwareId);
    tgaex.versionNumber = readShort(fp);
    tgaex.versionLetter = readChar(fp);

    tgaex.keyColor = readLong(fp);

    tgaex.pixelRatioNumerator = readShort(fp);
    tgaex.pixelRatioDenominator = readShort(fp);

    tgaex.gammaNumerator = readShort(fp);
    tgaex.gammaDenominator = readShort(fp);

    tgaex.colorOffset = readLong(fp);
    tgaex.stampOffset = readLong(fp);
    tgaex.scanOffset = readLong(fp);

    tgaex.attributesType = readByte(fp);

    return 1;
}

void printRGB(unsigned long r,unsigned long g,unsigned long b,FILE * fp)
{
    fprintf(fp,"(%lu , %lu , %lu)",r,g,b);
}

void printRGBA(unsigned long r,unsigned long g,unsigned long b,unsigned long a,FILE * fp)
{
    fprintf(fp,"(%lu , %lu , %lu , %lu)",r,g,b,a);
}


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

void printColorData(unsigned long data,FILE * out)
{
    unsigned long r,g,b,a;
    int visible;
    int pixelDepth;

    pixelDepth = tgah.colorMapType == COLOR_MAPPED ? tgah.colorMapDepth : tgah.pixelDepth;

    /* implemented support for 15-bit images? */

    if(tgah.imageType == UNCOMPRESSED_BLACK_AND_WHITE)
        printGrayScaleRGB(data,out);
    else if(tgah.imageType == UNCOMPRESSED_TRUE_COLOR ||
            tgah.imageType == RUN_LENGTH_ENCODED_TRUE_COLOR ||
            tgah.colorMapType == COLOR_MAPPED){
        if(pixelDepth == 24){

            r = (data & (0xff << 16)) >> 16;
            g = (data & (0xff << 8)) >> 8;
            b = data & 0xff;

            printRGB(r,g,b,out);
        } else if(pixelDepth == 32){

            a = (data & (0xff << 24)) >> 24;
            r = (data & (0xff << 16)) >> 16;
            g = (data & (0xff << 8)) >> 8;
            b = data & 0xff;

            printRGBA(r,g,b,a,out);
        } else if(pixelDepth == 16){

            r = (data & (0x1f << 10)) >> 10;
            g = (data & (0x1f << 5)) >> 5;
            b = data & 0x1f;
            visible = (data & (0x01 << 16)) >> 16;

            /* TODO: confused to meaning of the value of the alpha channel?*/
            a = visible ? 255 : 0;
            printRGBA(r,g,b,a,out);
        }
    }
}

void printcompressedImage(SHORT width,
                          SHORT height,
                          FILE * in,
                          FILE * out)
{

    /* it is assumed that the encoding always fit on a line */
    unsigned long data;
    BYTE length,head;
    int pixels = width * height;
    int i = 0;

    size_t pixelDepth = tgah.pixelDepth;


    for(i = 0; i < pixels; ++i){

       head = readByte(in);

        /* run length packet */
        if(head & 0x80){

            length = head & 0x7f;
            data = 0;
            fread(&data, pixelDepth / 8, 1, in);


            if(tgah.colorMapType == COLOR_MAPPED)
                data = colorMap[data];

            for(length = length + 1; length > 0; --length){
                /* TODO: replace with more generic method */
                printColorData(data,out);
            }

        } else {

            /* raw packet */
            length = head & 0x7f;
            for(length = length + 1; length > 0; --length){

		data = 0;
                fread(&data, pixelDepth / 8, 1, in);

                if(tgah.colorMapType == COLOR_MAPPED)
                    data = colorMap[data];

                printColorData(data,out);
            }
        }
    }
}

void printImage(SHORT width,
                SHORT height,
                FILE * in,
                FILE * out)
{
    SHORT row,col;
    unsigned long data;

    size_t pixelDepth = tgah.pixelDepth;

    for(row = 0; row < height; ++row){
        for(col = 0; col < width; ++col){
            data  = 0;
            fread(&data, pixelDepth / 8, 1, in);

            if(tgah.colorMapType == COLOR_MAPPED)
                data = colorMap[data];
            printColorData(data,out);
        }
        fprintf(out,"\n");
    }
}

void readStamp(LONG offset,FILE * in,FILE * out)
{
    BYTE width,height;
    fseek(in,offset,SEEK_SET);

    width = readByte(in);
    height = readByte(in);

    fprintf(out,"Width:%d\n",width);
    fprintf(out,"Height:%d\n",height);

    printImage(width,height,in,out);
}

void readColorMap(FILE * in,FILE * out)
{
    size_t pixelDepth = tgah.colorMapDepth;
    unsigned long data;
    int i;

    colorMap = (unsigned long *) malloc(tgah.colorMapLength * tgah.colorMapDepth);

    for(i = 0; i <  tgah.colorMapLength; ++i){

        data  = 0;
        fread(&data, pixelDepth / 8, 1, in);
        colorMap[i] = data;
        printColorData(data,out);
    }

    fprintf(out,"\n");
}
