#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tga.h"

/* strip the file extension of a filename. */
static void strip_extension(char * fileName);

/* getbits:  get n bits from position p */
static unsigned getbits(unsigned x, int p, int n);

/* Read a short from a file. */
static SHORT readShort(FILE * fp);

/* Read a byte from a file. */
static BYTE readByte(FILE * fp);

static void getImageDestStr(char * str,int imageOrigin);

int main(int argc, char *argv[])
{
    char s[2];
    CHAR b1 = 61;
    CHAR b2 = 54;
    s[0] = b1;
    s[1] = b2;
    s[2] = '\0';
    puts(s);
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

    in = fopen(file,"rb");

    if(in == NULL){
        perror("An error occurred opening the file");
        exit(1);
    }

    TGAHeader tgah;

//    fread(&tgah,sizeof(TGAHeader),1,in);
    tgah.IDLength = readByte(in);
    tgah.colorMapType = readByte(in);
    tgah.imageType = readByte(in);
    tgah.colorMapStart = readShort(in);
    tgah.colorMapLength = readShort(in);
    tgah.colorMapDepth = readByte(in);
    tgah.xOrigin = readShort(in);
    tgah.yOrigin = readShort(in);
    tgah.width = readShort(in);
    tgah.height = readShort(in);
    tgah.pixelDepth = readByte(in);
    tgah.imageDescriptor = readByte(in);


    /* Works because all tga files has a tga extension,
       meaning that ".dmp" will fit in the rest of the file.*/
    strip_extension(file);
    strcat(file,".dmp");

    out = fopen(file,"wb");

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
}

static void strip_extension(char * fileName)
{
    char * extBeg;
    int n;
    extBeg = strrchr(fileName,'.');
    if(extBeg == NULL);
    //return extBeg;
    else{
        n = extBeg - fileName;
        fileName[n] = '\0';
    }
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

static BYTE readByte(FILE * fp)
{
    BYTE s;
    fread(&s,sizeof(BYTE),1,fp);
    return s;
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
