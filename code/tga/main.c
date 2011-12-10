#include "main.h"

/* FIXME: Make sure alpha channels are printed out properly! */
/* TODO: Delete any unused function definitions from the header. */

int main(int argc, char * argv[])
{
    FILE * in;
    FILE * out;

    char * inName;
    char * outName;

    TGA_Image image;

    if(argc == 1){
        printf("No file was specified for loading\n");
        printf("Try --help for more information.\n");
        return 1;
    }

    ++argv;
    --argc;
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
        --argc;
    }

    if(argc < 1){
        printf("An input file must be specified.\n");
        return 1;
    }

    inName = argv[0];
    outName = argv[1];

    in = fopen(inName,"rb");
    assertFileOpened(in);

    out = fopen(outName,"wb");
    assertFileOpened(out);

    image = loadTGA_Image(in);
    writeTGA_Image(image,out);
    freeTGA_Image(image);

    fclose(in);
    fclose(out);

    return 0;
}

void printHelp(void)
{
    printf("Usage: tga IN OUT\n");
    printf("Dump the information and color data of a TGA file.\n\n");
    printf("  --help\tDisplay this help message.\n");

}

TGA_Image newTGA_Image(void)
{
    TGA_Image image;

    image.colorMap = NULL;
    image.extensionArea = NULL;

    image.colorData.data = NULL;
    image.postageStamp.data = NULL;

    return image;
}

void freeTGA_Image(TGA_Image image)
{
    if(image.extensionArea != NULL)
        free(image.extensionArea);

    if(image.colorMap != NULL)
        free(image.colorMap);

    if(image.colorData.data != NULL)
        free(image.colorData.data);

    if(image.postageStamp.data != NULL)
        free(image.postageStamp.data);

    image.colorMap = NULL;
    image.extensionArea = NULL;

    image.colorData.data = NULL;
    image.postageStamp.data = NULL;
}

void writeTGA_Image(TGA_Image image, FILE * out)
{
    fprintf(out,"Image Header:\n");
    verbosePrint("Writing Image Header\n");
    writeTGA_Header(image.header,out);

    if(image.extensionArea != NULL){
        fprintf(out,"Version:%s\n","2.0");
	verbosePrint("Writing Extension Area\n");
	fprintf(out,"Extension Area:\n");
        writeTGA_ExtensionArea(image.extensionArea,out);
    } else
        fprintf(out,"Version:%s\n","1.0");

    if(image.header.IDLength > 0){
	verbosePrint("Writing Image ID\n");
        fprintf(out,"Image ID:%s\n",image.imageID);
    }


    if(image.header.colorMapType == COLOR_MAPPED){
	verbosePrint("Writing Color Map\n");
	fprintf(out,"Color Map:\n");
        writeColorMap(image.header,image.colorMap,out);
    }

    fprintf(out,"Color data:\n");
    verbosePrint("Writing Color Data\n");
    writeTGA_ImageData(image.colorData,image.header,out);


    if(image.extensionArea != NULL && image.extensionArea->stampOffset != 0){

	verbosePrint("Writing Stamp Postage\n");

        fprintf(out,"Stamp postage:\n");
        fprintf(out,"Width:%d\n",image.postageStamp.width);
        fprintf(out,"Height:%d\n",image.postageStamp.width);
        writeTGA_ImageData(image.postageStamp,image.header,out);
    }
}


TGA_Image loadTGA_Image(FILE * in)
{
    TGA_Image image;
    int compressed;

    image = newTGA_Image();

    image.extensionArea = loadTGAExtensionArea(in);
    rewind(in);

    verbosePrint("Loading Header\n");
    image.header = loadTGA_Header(in);

    if(image.header.IDLength > 0){
        verbosePrint("Loading Image ID\n");

        readStr(in,image.header.IDLength * sizeof(char),image.imageID);

        /* Necessary? */
        image.imageID[image.header.IDLength] = '\0';
    }

    if(image.header.colorMapType == COLOR_MAPPED){
        verbosePrint("Loading Color Map\n");
        image.colorMap = loadColorMap(image.header,in);
    }

    compressed =
        image.header.imageType == RUN_LENGTH_ENCODED_BLACK_AND_WHITE ||
        image.header.imageType == RUN_LENGTH_ENCODED_TRUE_COLOR ||
        image.header.imageType == RUN_LENGTH_ENCODED_COLOR_MAPPED;

    verbosePrint("Loading Color Data\n");
    image.colorData = loadTGA_ImageData(
	image.header.width, image.header.height,
	image.header,image.colorMap,compressed,in);

    if(image.extensionArea != NULL && image.extensionArea->stampOffset != 0){

	verbosePrint("Loading Postage Stamp\n");

        image.postageStamp = loadPostageStamp(
            image.header,
            image.colorMap,
            image.extensionArea->stampOffset,
            in);
    }

    return image;
}

TGA_Header loadTGA_Header(FILE * in)
{
    TGA_Header header;

    header.IDLength = readByte(in);
    header.colorMapType = readByte(in);
    header.imageType = readByte(in);
    header.colorMapStart = readShort(in);
    header.colorMapLength = readShort(in);
    header.colorMapDepth = readByte(in);
    header.xOrigin = readShort(in);
    header.yOrigin = readShort(in);
    header.width = readShort(in);
    header.height = readShort(in);
    header.pixelDepth = readByte(in);
    header.imageDescriptor = readByte(in);

    return header;
}

void writeTGA_Header(TGA_Header header, FILE * out)
{
    BYTE alphaChannelBits;

    alphaChannelBits = getbits(header.imageDescriptor,3,4);

    fprintf(out,"Id Length:%d\n",header.IDLength);
    fprintf(out,"Color map type:%d\n",header.colorMapType);
    fprintf(out,"Image type:%d\n",header.imageType);
    fprintf(out,"Color map start:%d\n",header.colorMapStart);
    fprintf(out,"Color map length:%d\n",header.colorMapLength);
    fprintf(out,"Color map depth:%d\n",header.colorMapDepth);
    fprintf(out,"X origin:%d\n",header.xOrigin);
    fprintf(out,"Y origin:%d\n",header.yOrigin);
    fprintf(out,"Width:%d\n",header.width);
    fprintf(out,"Height:%d\n",header.height);
    fprintf(out,"pixelDepth:%d\n",header.pixelDepth);
    fprintf(out,"Image Descriptor:%d\n",header.imageDescriptor);
    fprintf(out,"Alpha Channel Bits:%d\n",alphaChannelBits);
    fprintf(out,"Screen destination of first pixel: ");

    if((1 << 5) & header.imageDescriptor)
        fprintf(out,"Top ");
    else
        fprintf(out,"Bottom ");

    if((1 << 4) & header.imageDescriptor)
        fprintf(out,"right\n");
    else
        fprintf(out,"left\n");

}


void writeColorMap(TGA_Header header, unsigned long * colorMap, FILE * out)
{
    int i;

    fprintf(out,"Color Map:\n");

    for(i = header.colorMapStart; i <  header.colorMapLength; ++i)
        writeColorData(colorMap[i], header,out);

    fprintf(out,"\n\n");
}

void writeTGA_ExtensionArea(TGA_ExtensionArea * extension, FILE * out)
{
    fprintf(out,"Extension Size:%d\n",extension->size);
    fprintf(out,"Author Name:%s\n",extension->authorName);

    fprintf(out,"Author Comment:\n");
    printFormatAuthorComment(extension->authorComment,out);
    fprintf(out,"End Author Comment:\n");

    fprintf(out,"Stamp date/time: %d/%d - %d %d:%d:%d\n",
            extension->stampDay,
            extension->stampMonth,
            extension->stampYear,
            extension->stampHour,
            extension->stampMinute,
            extension->stampSecond);

    fprintf(out,"Job name: %s\n",extension->jobName);

    fprintf(out,"Job time: %d:%d:%d\n",
            extension->jobHour,
            extension->jobMinute,
            extension->jobSecond);

    fprintf(out,"Software ID: %s",extension->softwareId);

    if(extension->versionNumber != 0 ){
        fprintf(out," %.2f%c\n",extension->versionNumber / 100.0,extension->versionLetter);
    }else{
        fprintf(out,"\n");
    }

    fprintf(out,"Key color: %d\n",extension->keyColor);

    fprintf(out,"Pixel Ratio Numerator: %d\n",extension->pixelRatioNumerator);
    fprintf(out,"Pixel Ratio Denominator: %d\n",extension->pixelRatioDenominator);

    fprintf(out,"Gamma value:");
    if(extension->gammaDenominator == 0){
        fprintf(out,"Unused\n");
    } else{
        fprintf(out,"%f\n",(float)extension->gammaNumerator / (float)extension->gammaDenominator);
    }

    fprintf(out,"color correction offset: %d\n",extension->colorOffset);
    fprintf(out,"Postage stamp offset: %d\n",extension->stampOffset);
    fprintf(out,"Scan line offset: %d\n",extension->scanOffset);

    fprintf(out,"Attributes Type: %d\n",extension->attributesType);
}

TGA_ExtensionArea * loadTGAExtensionArea(FILE * in)
{
    char signature[18];
    LONG extensionAreaOffset;
    TGA_ExtensionArea * extensionArea;

    /* Check the footer. */

    fseek(in, -26, SEEK_END);
    fseek(in, 8, SEEK_CUR);

    readStr(in,18,signature);

    /* It's not the proper signature, so there's no extension area. */
    if(strcmp(signature,"TRUEVISION-XFILE."))
        return NULL;

    /* Else, read the extension area.*/
    fseek(in, -26, SEEK_END);


    /* Read the extension area offset. */

    fread(&extensionAreaOffset,sizeof(LONG),1,in);

    if(extensionAreaOffset == 0){
        /* There is no extension area. */
        return NULL;
    }

    verbosePrint("Loading the extension Area. Signature: %d, Offset: %d\n",
                 signature,
                 extensionAreaOffset);

    extensionArea = (TGA_ExtensionArea *) malloc(sizeof(TGA_ExtensionArea));

    fseek(in,extensionAreaOffset,SEEK_SET);

    /* read the extension area */

    extensionArea->size = readShort(in);
    readStr(in,41,extensionArea->authorName);
    readStr(in,324,extensionArea->authorComment);

    extensionArea->stampMonth = readShort(in);
    extensionArea->stampDay = readShort(in);
    extensionArea->stampYear = readShort(in);
    extensionArea->stampHour = readShort(in);
    extensionArea->stampMinute = readShort(in);
    extensionArea->stampSecond = readShort(in);

    readStr(in,41,extensionArea->jobName);
    extensionArea->jobHour = readShort(in);
    extensionArea->jobMinute = readShort(in);
    extensionArea->jobSecond = readShort(in);

    readStr(in,41,extensionArea->softwareId);
    extensionArea->versionNumber = readShort(in);
    extensionArea->versionLetter = readByte(in);

    extensionArea->keyColor = readLong(in);

    extensionArea->pixelRatioNumerator = readShort(in);
    extensionArea->pixelRatioDenominator = readShort(in);

    extensionArea->gammaNumerator = readShort(in);
    extensionArea->gammaDenominator = readShort(in);

    extensionArea->colorOffset = readLong(in);
    extensionArea->stampOffset = readLong(in);
    extensionArea->scanOffset = readLong(in);

    extensionArea->attributesType = readByte(in);

    return extensionArea;
}

void printRGB(unsigned long r,unsigned long g,unsigned long b,FILE * in)
{
    fprintf(in,"(%lu,%lu,%lu)",r,g,b);
}

void printRGBA(unsigned long r,unsigned long g,unsigned long b,unsigned long a,FILE * fp)
{
    fprintf(fp,"(%lu,%lu,%lu,%lu)",r,g,b,a);
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


void writeColorData(unsigned long data,TGA_Header header,FILE * out)
{
    unsigned long r,g,b,a;
    int visible;
    int pixelDepth;
    int isGrayScale;

    pixelDepth = header.colorMapType == COLOR_MAPPED ? header.colorMapDepth : header.pixelDepth;

    /* implement support for 15-bit images? */

    isGrayScale = (header.imageType == UNCOMPRESSED_BLACK_AND_WHITE ||
                   header.imageType == RUN_LENGTH_ENCODED_BLACK_AND_WHITE);

    if(isGrayScale)
        printGrayScaleRGB(data,out);
    else{
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

void writeTGA_ImageData(TGA_ImageData image, TGA_Header header,FILE * out)
{
    int length;
    int i = 0;

    length = image.width * image.height;

    for(i = 0; i < length; ++i){
        writeColorData(image.data[i],header,out);

        if((i + 1) % image.width == 0)
            fprintf(out,"\n\n");
    }
}

TGA_ImageData loadTGA_ImageData(SHORT width, SHORT height,TGA_Header header,unsigned long * colorMap, int compressed, FILE * in)
{

    /* it is assumed that the encoding always fit on a line */
    unsigned long data;
    BYTE length,head;
    int pixels;
    int i = 0;
    size_t pixelDepth = header.pixelDepth;

    TGA_ImageData image;

    pixels = width * height;


    image.width = width;
    image.height = height;
    image.data = (unsigned long *) malloc(sizeof(unsigned long) * pixels);

    while(i < pixels){

        if(compressed){
            head = readByte(in);

            length = head & 0x7f;

            /* run length packet */
            if(head & 0x80){

                data = 0;
                fread(&data, pixelDepth / 8, 1, in);

                if(header.colorMapType == COLOR_MAPPED)
                    data = colorMap[data];

                for(length = length + 1; length > 0; --length){
                    image.data[i] = data;
                    ++i;
                }

            } else {

                /* raw packet */
                for(length = length + 1; length > 0; --length){

                    data = 0;
                    fread(&data, pixelDepth / 8, 1, in);

                    if(header.colorMapType == COLOR_MAPPED)
                        data = colorMap[data];


                    image.data[i] = data;

                    ++i;
                }
            }
        } else {
            data  = 0;
            fread(&data, pixelDepth / 8, 1, in);

            if(header.colorMapType == COLOR_MAPPED)
                data = colorMap[data];


            image.data[i] = data;

            ++i;
            /* newline handling */
        }
    }

    return image;
}

TGA_ImageData loadPostageStamp(TGA_Header header,unsigned long * colorMap, LONG offset,FILE * in)
{
    BYTE width,height;

    fseek(in,offset,SEEK_SET);

    width = readByte(in);
    height = readByte(in);

    return loadTGA_ImageData(width,height,header,colorMap,0,in);
}


unsigned long * loadColorMap(TGA_Header header, FILE * in)
{
    size_t pixelDepth = header.colorMapDepth;
    unsigned long data;
    int i;

    unsigned long * colorMap;

    colorMap = (unsigned long *) malloc(sizeof(unsigned long) * header.colorMapLength * header.colorMapDepth);

    fseek(in,header.colorMapStart,SEEK_CUR);

    for(i = header.colorMapStart; i <  header.colorMapLength; ++i){

        data  = 0;
        fread(&data, pixelDepth / 8, 1, in);
        colorMap[i] = data;
    }

    return colorMap;
}

unsigned getbits(unsigned x, int p, int n)
{
    return (x >> (p+1-n)) & ~(~0 << n);
}

SHORT readShort(FILE * fp)
{
    SHORT s;
    fread(&s,sizeof(SHORT),1,fp);
    return s;
}

LONG readLong(FILE * fp)
{
    LONG s;
    fread(&s,sizeof(LONG),1,fp);
    return s;
}

