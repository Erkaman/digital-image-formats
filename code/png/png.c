#include "png.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

/* FIXME: Check the output of basn3p04.png in ./png and netpbm. Fix it. */

void dumpPNG(FILE * in, FILE * out)
{
    PNG_Image image;

    image = loadPNG(in);
    writePNG(image,out);
    freePNG_Image(image);
}

void freePNG_Image(PNG_Image image)
{
    if(image.renderingIntent != NULL)
        free(image.renderingIntent);

    if(image.imageGamma != NULL)
        free(image.imageGamma);

    if(image.timeStamp != NULL)
        free(image.timeStamp);

    if(image.backgroundColor != NULL)
        free(image.backgroundColor);

    if(image.pixelDimensions != NULL)
        free(image.pixelDimensions);

    if(image.significantBits != NULL)
        free(image.significantBits);


    if(image.transparency != NULL){
        freeDataList(image.transparency->transparentIndices, 1);
        free(image.transparency);
    }

    freeDataList(image.textDataList,1);
    freeDataList(image.colorData,1);

    if(image.palette != NULL){
        freeDataList(*image.palette,1);
        free(image.palette);
    }
}

void loadSignature(BYTE * signature, FILE * in)
{
    fread(signature, sizeof(BYTE),SIGNATURE_LENGTH , in);
}

PNG_Image getEmptyPNG_Image(void)
{
    PNG_Image image;

    image.profile = NULL;
    image.chromaticities = NULL;
    image.imageGamma = NULL;
    image.significantBits = NULL;
    image.renderingIntent = NULL;
    image.backgroundColor = NULL;
    image.imageHistogram = NULL;
    image.pixelDimensions = NULL;
    image.timeStamp = NULL;
    image.transparency = NULL;
    image.palette = NULL;

    image.textDataList = getNewDataList(freeTextualData, copyByte);

    return image;
}

void freeTextualData(void * textualData)
{
    TextualData * data;

    data = (TextualData *) textualData;

    if(data->str != NULL)
        free(data->str);

    free(data);
}

PNG_Image loadPNG(FILE * in)
{
    PNG_Image image;
    Chunk chunk;
    DataStream stream;

    image = getEmptyPNG_Image();

    loadSignature(image.signature, in);

    image.header = loadImageHeader(in);

    while(1){

        chunk = loadChunk(in);

        stream = getNewDataStream(chunk.data, PNG_ENDIAN);

        if(isChunkType(chunk, IEND))
            break;

        else if(isChunkType(chunk, PLTE)){
            image.palette = loadPalette(stream);
        }

        else if(isChunkType(chunk, IDAT)){

            /* Load the rest of the idat chunk, if there are any.. */

            image.colorData = loadColorData(chunk.data, image.header);

        }else if(isChunkType(chunk, sRGB))
            image.renderingIntent = loadRenderingIntent(stream);

        else if(isChunkType(chunk, gAMA))
            image.imageGamma = loadImageGamma(stream);

        else if(isChunkType(chunk, pHYs))
            image.pixelDimensions = loadPixelDimensions(stream);

        else if(isChunkType(chunk, tIME))
            image.timeStamp = loadTimeStamp(stream);

        else if(isChunkType(chunk, bKGD))
            image.backgroundColor = loadBackgroundColor(image.header, stream);

        else if(isChunkType(chunk, tRNS))
            image.transparency = loadTransparency(image.header, stream);

        else if(isChunkType(chunk, tEXt))
            addToDataList(
                &image.textDataList,
                loadTextualData(stream, 0, chunk.length));

        else if(isChunkType(chunk, sBIT))
            image.significantBits = loadSignificantBits(image.header, stream);

        else {
            if(!isCriticalChunk(chunk)){
                printWarning("Unknown ancillary chunk %s found, skipping chunk.\n",
                             chunk.type);
            } else{
                printError("Unknown critcal chunk %s found.\n", chunk.type);
/*                exit(1); */
            }
        }

        freeChunk(chunk);
    }

    return image;
}

int isChunkType(Chunk chunk, char * chunkType)
{
    return (!strcmp(chunk.type, chunkType));
}

void freeChunk(Chunk chunk)
{
    freeDataList(chunk.data,1);
}

void writePNG(PNG_Image image, FILE * out)
{
    writeSignature(image.signature, out);
    writeHeader(image.header, out);

    writeRenderingIntent(image.renderingIntent, out);
    writeImageGamma(image.imageGamma, out);
    writePixelDimensions(image.pixelDimensions, out);
    writeTimeStamp(image.timeStamp, out);
    writeBackgroundColor(image, image.backgroundColor, out);
    writeSignificantBits(image.significantBits, image.header, out);

    writeTextDataList(image.textDataList, out);

    writePalette(image, out);

    writeColorData(image, out);

}

void writeRenderingIntent(BYTE * renderingIntent, FILE * out)
{
    if(renderingIntent != NULL){
        fprintf(out,"Rendering Intent: ");

        switch(*renderingIntent){
        case PERCEPTUAL_RENDERING_INTENT:
            fprintf(out, "Perceptual");
            break;
        case RELATIVE_COLORIMETRIC_RENDERING_INTENT:
            fprintf(out, "Relative Colorimetric");
            break;
        case SATURATION_RENDERING_INTENT:
            fprintf(out, "Saturation");
            break;
        case ABSOLUTE_COLORIMETRIC_RENDERING_INTENT:
            fprintf(out, "Absolute colorimetric");
            break;
        }

        fprintf(out, "\n");
    }
}

void writeHeader(ImageHeader header, FILE * out)
{
    fprintf(out, "PNG Image Header:\n");

    fprintf(out, "Image width: %u\n", header.width);
    fprintf(out, "Image height: %u\n", header.height);

    fprintf(out, "Bit Depth: %u bits\n", header.bitDepth);
    fprintf(out, "Color type: ");

    switch(header.colorType){
    case GREYSCALE_COLOR:
        fprintf(out, "Greyscale");
        break;
    case TRUECOLOR_COLOR:
        fprintf(out, "Truecolor");
        break;
    case INDEXED_COLOR:
        fprintf(out, "Indexed Color");
        break;
    case GREYSCALE_ALPHA_COLOR:
        fprintf(out, "Greyscale Alpha Color");
        break;
    case TRUECOLOR_ALPHA_COLOR:
        fprintf(out, "Truecolor Alpha Color");
        break;
    }

    fprintf(out, "(%u)\n",header.colorType);

    fprintf(out, "Compression Method: %u(always deflate if 0)\n", header.compressionMethod);
    fprintf(out, "Filter Method: %u(always 0, which is default filtering)\n", header.filterMethod);

    fprintf(out, "Interlace Method:");

    if(header.interlaceMethod == NO_INTERLACE)
        fprintf(out,"No interlacing");
    else if(header.interlaceMethod == ADAM7_INTERLACE)
        fprintf(out,"Adam7 interlacing");

    fprintf(out, "(%u)\n",header.interlaceMethod);
}

void writeSignature(BYTE * signature, FILE * out)
{
    size_t i;

    fprintf(out, "PNG Signature:\n");
    for(i = 0; i < SIGNATURE_LENGTH; ++i)
        if(i != (SIGNATURE_LENGTH - 1))
            fprintf(out, "%d ", signature[i]);
        else
            fprintf(out, "%d\n", signature[i]);
}

Chunk loadChunk(FILE * in)
{
    Chunk chunk;

    verbosePrint("Loading chunk...\n");

    fread(&chunk.length, sizeof(INT32), 1, in);
    chunk.length = htonl(chunk.length);

    verbosePrint("Chunk length: % ld\n", chunk.length);

    fread(chunk.type, sizeof(char), 4, in);
    chunk.type[4] = '\0';
    verbosePrint("Chunk Type: %s\n", chunk.type);

    /* TODO: Remember to free this memory! */
    chunk.data = readBytes(chunk.length, in);

    verbosePrint("data:\n");
    printDataList(chunk.data, printByte);

    fread(&chunk.CRC, sizeof(INT32), 1, in);
    chunk.CRC = htonl(chunk.CRC);
    verbosePrint("Chunk CRC: %ld\n", chunk.CRC);

    validateCRC(chunk);

    return chunk;
}


/* Some kind of copying routine is needed! */
void validateCRC(Chunk chunk)
{
    DataList checkData;

    INT32 calcCRC;
    BYTE b;
    size_t i;

    checkData = getNewDataList(NULL, copyByte);

    /* Convert the chunk type string to an array of bytes. */

    for(i = 0; i < 4; ++i){
        b = chunk.type[i];
        addByteToDataList(&checkData, b);
    }

    copyAppendToDataList(&checkData, chunk.data);

    calcCRC = crc32(checkData);

    freeDataList(checkData, 1);

    if(calcCRC != chunk.CRC){
        printError("Chunk has invalid checksum: chunk %d != calc %d\n", chunk.CRC, calcCRC );
        exit(1);
    } else
        verbosePrint("Chunk has valid checksum!\n");

}

int isCriticalChunk(Chunk chunk)
{
    return isupper((BYTE)chunk.type[0]);
}

ImageHeader loadImageHeader(FILE * in)
{
    ImageHeader header;
    Chunk headerChunk;
    DataStream stream;

    headerChunk = loadChunk(in);
    stream = getNewDataStream(headerChunk.data, PNG_ENDIAN);

    header.width = read32BitsNumber(&stream);
    header.height = read32BitsNumber(&stream);
    header.bitDepth = readStreamByte(&stream);
    header.colorType = readStreamByte(&stream);
    header.compressionMethod = readStreamByte(&stream);
    header.filterMethod = readStreamByte(&stream);
    header.interlaceMethod = readStreamByte(&stream);

    freeChunk(headerChunk);

    return header;
}

BYTE * loadRenderingIntent(DataStream stream)
{
    BYTE * b;

    b = malloc(sizeof(BYTE));

    *b = readStreamByte(&stream);

    return b;
}

#define CRC32_POLY 0xEDB88320

unsigned int crc32(DataList data){

    unsigned long reminder = 0xFFFFFFFF; /* standard initial value in CRC32 */
    unsigned long i;
    unsigned long bit;
    BYTE b;

    for(i = 0; i < data.count; ++i){
        b = *(BYTE *)data.list[i];
        reminder ^= b; /* must be zero extended */
        for(bit = 0; bit < 8; bit++)
            if(reminder & 0x01)
                reminder = (reminder >> 1) ^ CRC32_POLY;
            else
                reminder >>= 1;
    }

    return reminder ^ 0xFFFFFFFF;
}

INT32 * loadImageGamma(DataStream stream)
{
    INT32 * gamma;

    gamma = malloc(sizeof(INT32));

    *gamma = read32BitsNumber(&stream);

    return gamma;
}
void writeImageGamma(INT32 * imageGamma, FILE * out)
{
    if(imageGamma != NULL){
        fprintf(out, "Image Gamma: %d / 100000\n", *imageGamma);
    }
}

PixelDimensions * loadPixelDimensions(DataStream stream)
{
    PixelDimensions * pixelDimensions;

    pixelDimensions = malloc(sizeof(PixelDimensions));

    pixelDimensions->x = read32BitsNumber(&stream);
    pixelDimensions->y = read32BitsNumber(&stream);

    pixelDimensions->unitSpecifier = readStreamByte(&stream);

    return pixelDimensions;
}

void writePixelDimensions(PixelDimensions * pixelDimensions, FILE * out)
{
    char unit[] = "Unknown unit";


    if(pixelDimensions != NULL){

        fprintf(out, "Pixel Dimensions:\n");

        if(pixelDimensions->unitSpecifier == METRE_UNIT)
            strcpy(unit,"Metre");

        fprintf(out, "X:%d pixels per %s\n",pixelDimensions->x, unit);
        fprintf(out, "Y:%d pixels per %s\n",pixelDimensions->y, unit);
    }
}

TimeStamp * loadTimeStamp(DataStream stream)
{
    TimeStamp * timeStamp;

    timeStamp = malloc(sizeof(TimeStamp));

    timeStamp->year = read16BitsNumber(&stream);

    timeStamp->month = readStreamByte(&stream);
    timeStamp->day = readStreamByte(&stream);
    timeStamp->hour = readStreamByte(&stream);
    timeStamp->minute = readStreamByte(&stream);
    timeStamp->second = readStreamByte(&stream);

    return timeStamp;
}

void writeTimeStamp(TimeStamp * timeStamp, FILE * out)
{
    if(timeStamp != NULL){
        fprintf(out,"Image last-modification time: ");

        fprintf(out, "%d/%d - %d %02d:%02d:%02d\n",
                timeStamp->day,
                timeStamp->month,
                timeStamp->year,
                timeStamp->hour,
                timeStamp->minute,
                timeStamp->second
            );
    }
}

Color * loadBackgroundColor(ImageHeader header, DataStream stream)
{
    Color * backgroundColor;

    backgroundColor = malloc(sizeof(Color));

    /*

      "The background colour given by the bKGD chunk is not to be
      considered transparent"

      And therefore, the value of the alpha channel will always
      be its maximum value.

    */

    /* REDUNDANT CODE.*/

    if(header.colorType == GREYSCALE_COLOR){

        backgroundColor->greyscale = read16BitsNumber(&stream);

    } else if(header.colorType == GREYSCALE_ALPHA_COLOR){

        backgroundColor->greyscaleAlpha.greyscale = read16BitsNumber(&stream);
        backgroundColor->greyscaleAlpha.alpha = read16BitsNumber(&stream);

        backgroundColor->greyscaleAlpha.alpha =
            getMaximumChannelValue(header);
    }

/* convert -size 1x1 xc:transparent -fill 'rgba(180, 180, 180, 0.8)' -draw 'rectangle 0,0 1,1' bgbox.png
 */

    return backgroundColor;
}

INT32 getMaximumChannelValue(ImageHeader header)
{
    return pow(2, header.bitDepth) - 1;
}


void writeBackgroundColor(
    PNG_Image image,
    Color * backgroundColor,
    FILE * out)
{
    if(backgroundColor != NULL){
        fprintf(out,"Background color: ");
        writeColor(image,*backgroundColor, out);
        fprintf(out,"\n");
    }
}

void writeColor(PNG_Image image,
                Color color,
                FILE * out)
{
    Color indexedColor;
    BYTE alpha;

    verbosePrint("colorType:%d\n", image.header.colorType);

    if(image.header.colorType == GREYSCALE_COLOR){

        if(image.transparency != NULL){

            if(image.transparency->color.greyscale == color.greyscale)
                fprintf(out, "(transparent)");
            else
                fprintf(out, "(%d)",color.greyscale);


        } else
            fprintf(out, "(%d)",color.greyscale);
    }

    else if(image.header.colorType == GREYSCALE_ALPHA_COLOR){

        fprintf(out, "(%d,%d)",color.greyscaleAlpha.greyscale,color.greyscaleAlpha.alpha);
    }

    else if(image.header.colorType == TRUECOLOR_COLOR){

        if(image.transparency != NULL){
            printf("To be implemented, bitch.\n");
            exit(1);
        }


        fprintf(out, "(%d,%d,%d)",
                color.rgb.R,
                color.rgb.G,
                color.rgb.B);

    }

    else if(image.header.colorType == TRUECOLOR_ALPHA_COLOR){

        fprintf(out, "(%d,%d,%d,%d)",
                color.rgba.R,
                color.rgba.G,
                color.rgba.B,
                color.rgba.A);
    }

    else if(image.header.colorType == INDEXED_COLOR){

        indexedColor = *(Color *)image.palette->list[color.index];

        if(image.transparency != NULL){

            /* If the color is not there, it is 255 by default? check png spec*/
            alpha = *(BYTE*)image.transparency->transparentIndices.list[color.index];

            fprintf(out, "(%d,%d,%d,%d)",
                    indexedColor.rgb.R,
                    indexedColor.rgb.G,
                    indexedColor.rgb.B,
                    alpha);


        } else {
            fprintf(out, "(%d,%d,%d)",
                    indexedColor.rgb.R,
                    indexedColor.rgb.G,
                    indexedColor.rgb.B);

        }
    }

}

TextualData * loadTextualData(
    DataStream stream,
    int compressed,
    INT32 chunkLength)
{
    TextualData * data;
    char ch;
    int i;
    int textLength;


    data = malloc(sizeof(TextualData));
    data->str = NULL;

    /* Read the keyword. */
    i = 0;
    do{
        ch = (char)readStreamByte(&stream);

        data->keyword[i++] = ch;

    } while(ch != '\0');


    if(compressed){
        /* Uncompress the data.*/
    } else{
        /* Simply read the data.

         */

        textLength = chunkLength - i;

        /* +1 makes space for the null character. */
        data->str = malloc(sizeof(char) * (textLength + 1));

        for(i = 0; i < (textLength); ++i){
            data->str[i] = (char)readStreamByte(&stream);
        }

        data->str[i] = '\0';
    }

    return data;
}

void writeTextDataList(DataList textDataList, FILE * out)
{
    size_t i;
    TextualData * data;
    if(textDataList.count > 0){

        for(i = 0; i < textDataList.count; ++i){
            fprintf(out, "Textual data %ld:\n", i+1);
            data = (TextualData *)textDataList.list[i];
            fprintf(out, "Keyword: %s\n", data->keyword);

            fprintf(out, "Text: %s\n", data->str);
        }
    }
}

DataList loadColorData(DataList data, ImageHeader header)
{
    int former;
    DataList decompressed, unfiltered, colorData, interlaced;
    former = verbose;

    verbose = 0;
    decompressed = ZLIB_Decompress(data);
    verbose = former;

    /* Unfilter. */

    unfiltered = unfilter(decompressed, header);

    freeDataList(decompressed, 1);

    /* Undo the Scanline serialization; that is, split up the bytes into colors. */

    colorData = splitUpColorData( unfiltered, header);
    freeDataList(unfiltered, 1);

    if(header.interlaceMethod == ADAM7_INTERLACE){
        interlaced = uninterlace(colorData, header);
        freeDataList(colorData, 0);
        return interlaced;
    } else
        return colorData;
}

void writeColorData(PNG_Image image, FILE * out)
{
    size_t row,col;

    fprintf(out,"Color Data:\n");

    for(row = 0; row < image.header.height; ++row){

        for(col = 0; col < image.header.width; ++col){

            writeColor(image,*(Color *)image.colorData.list[row * image.header.width + col], out);
            fprintf(out,",");
        }

        fprintf(out,"\n\n");
    }

}

/* See: http://www.w3.org/TR/PNG-Filters.html*/
DataList unfilter(DataList data, ImageHeader header)
{
    DataList unfiltered;
    DataStream stream;
    size_t scanline;
    int filterType;
    size_t i;
    size_t width;

    size_t bpp;

    BYTE unfilteredByte;

    ColorInfo info;

    /* The byte being filtered. */
    BYTE x;

    /* The previous pixel channel pixel value. */
    BYTE a;

    /* The upper byte in the previous scanline. */
    BYTE b;

    BYTE c;

    info = getColorInfo(header);

    bpp =  (int)ceil((double)(info.numChannels * info.channelBitDepth) / 8.0);

    for(i = 0; i < data.count; ++i){
        verbosePrint("%d,", getByteAt(data, i));
    }
    verbosePrint("\n");

    width = (data.count / header.height) - 1;

    stream = getNewDataStream(data, PNG_ENDIAN);
    unfiltered = getNewDataList(NULL, copyByte);

    /* Read scanline after scanline*/
    for(scanline = 0; scanline < header.height; ++scanline){

        /* Read the filter type. */
        filterType = readStreamByte(&stream);

        verbosePrint("Filtertype: %d\n", filterType);

        for(i = 0; i < width; ++i){

            x = readStreamByte(&stream);

            /* Unfilter byte according to a, b, c and x*/

            if(filterType == NO_FILTER)
                unfilteredByte = x;

            else if(filterType == SUB_FILTER){

                a = compute_a(i, bpp, unfiltered);

                unfilteredByte = (BYTE)((unsigned int)x + (unsigned int)a) % 256;
            }

            else if(filterType == UP_FILTER){

                /* "On the first scanline of an image (or of a pass of
                   an interlaced image), assume Prior(x) = 0 for all x." */
                b = compute_b(scanline, width, unfiltered);

                unfilteredByte = (BYTE)((unsigned int)x + (unsigned int)b) % 256;
            }

            else if(filterType == AVERAGE_FILTER){

                a = compute_a(i, bpp, unfiltered);
                b = compute_b(scanline, width, unfiltered);

                /* FIXME? Another mod 256 needed here? */
                unfilteredByte = (BYTE)((unsigned int)x +

                                        floor(((unsigned int)a + (unsigned int)b) / 2)
                    )% 256;

            } else if(filterType == PAETH_FILTER){

                a = compute_a(i, bpp, unfiltered);
                b = compute_b(scanline, width, unfiltered);
                c = compute_c(i, bpp, scanline, width, unfiltered);

                verbosePrint("a=%d,b=%d,c=%d\n", a , b , c);

                unfilteredByte = (BYTE)((unsigned int)x +
                                        paethPredictor(a,b,c)) % 256;

            }

            /* Add it.*/
            verbosePrint("unfiltered:%d\n", unfilteredByte);

            addByteToDataList(&unfiltered, unfilteredByte);
        }
    }

    return unfiltered;
}

ColorInfo getColorInfo(ImageHeader header)
{
    ColorInfo info;

    info.channelBitDepth = header.bitDepth;

    if(header.colorType == GREYSCALE_COLOR ||
       header.colorType == INDEXED_COLOR)
        info.numChannels = 1;
    if(header.colorType == TRUECOLOR_COLOR)
        info.numChannels = 3;
    if(header.colorType == GREYSCALE_ALPHA_COLOR)
        info.numChannels = 2;
    if(header.colorType == TRUECOLOR_ALPHA_COLOR)
        info.numChannels = 4;

    return info;
}

BYTE compute_a(size_t i, size_t bpp, DataList unfiltered)
{
    if(i >= bpp)
        return getByteAt(unfiltered , unfiltered.count - bpp);
    else
        return 0;
}

BYTE compute_b(size_t scanline, size_t width, DataList unfiltered)
{
    if(scanline == 0)
        return 0;
    else
        return getByteAt(unfiltered , unfiltered.count - width);
}

BYTE compute_c(
    size_t i,
    size_t bpp,
    size_t scanline,
    size_t width,
    DataList unfiltered)
{
    if(scanline == 0)
        return 0;
    else {

        if(i >= bpp)
            return getByteAt(unfiltered , unfiltered.count - width - bpp);
        else
            return 0;

    }
}

unsigned int paethPredictor(unsigned int a, unsigned int b, unsigned int c)
{
    unsigned int pa, pb, pc, p;

    p = a + b - c;

    pa = abs(p - a);
    pb = abs(p - b);
    pc = abs(p - c);

    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else
        return c;
}

DataList splitUpColorData(DataList data, ImageHeader header)
{
    size_t i;
    DataList colorData;
    Color color;
    DataStream colorStream;

    colorData = getNewDataList(NULL, NULL);

    colorStream = getNewDataStream(data, PNG_ENDIAN);

    for(i = 0; i < (header.width * header.height); ++i){

        if(header.colorType == GREYSCALE_COLOR){
            color.greyscale = readNextChannel(&colorStream, header);
        }

        else if(header.colorType == INDEXED_COLOR){
            color.index = readNextChannel(&colorStream, header);
        }

        else if(header.colorType == TRUECOLOR_COLOR){
            color.rgb.R = readNextChannel(&colorStream, header);
            color.rgb.G = readNextChannel(&colorStream, header);
            color.rgb.B = readNextChannel(&colorStream, header);
        }

        else if(header.colorType == GREYSCALE_ALPHA_COLOR){

            color.greyscaleAlpha.greyscale = readNextChannel(&colorStream, header);
            color.greyscaleAlpha.alpha = readNextChannel(&colorStream, header);
        }

        else if(header.colorType == TRUECOLOR_ALPHA_COLOR){

            color.rgba.R = readNextChannel(&colorStream, header);
            color.rgba.G = readNextChannel(&colorStream, header);
            color.rgba.B = readNextChannel(&colorStream, header);
            color.rgba.A = readNextChannel(&colorStream, header);
        }

        addColorToDataList(&colorData, color);

        if((i+1) % header.width == 0 && (i+1) != header.width * header.height &&
           header.bitDepth < 8){

            colorStream.b = readStreamByte(&colorStream);
            colorStream.remainingBitsBits = 8;

        }
    }

    return colorData;
}

unsigned long readNextChannel(DataStream * stream, ImageHeader header)
{
    unsigned long channel;

    if(header.bitDepth == 8){
        channel = readStreamByte(stream);
    } else if(header.bitDepth == 16){
        channel = read16BitsNumber(stream);
    } else if(header.bitDepth < 8)
        /* Handle bit depths 1, 2 and 4*/
        channel = readBits(stream, header.bitDepth);


    return channel;
}

void addColorToDataList(DataList * list, Color color)
{
    Color * cp;

    cp = malloc(sizeof(Color));
    *cp = color;
    addToDataList(list, cp);
}

DataList * loadPalette(DataStream stream)
{
    DataList * palette;
    size_t size;
    size_t i;
    Color color;

    palette = malloc(sizeof(DataList));
    *palette = getNewDataList(NULL, NULL);

    verbosePrint("size:%d\n",stream.list.count);
    size = stream.list.count / 3;

    verbosePrint("loading palette\n");
    for(i = 0; i < size; ++i){
        color.rgb.R = readStreamByte(&stream);
        color.rgb.G = readStreamByte(&stream);
        color.rgb.B = readStreamByte(&stream);

        verbosePrint("(%d,%d,%d)\n",
                     color.rgb.R,
                     color.rgb.G,
                     color.rgb.B);

        addColorToDataList(palette, color);
    }

    return palette;
}

void writePalette(PNG_Image image,FILE * out)
{
    image = image;
    out = out;
/*    size_t i;

      if(image.palette.count > 0){
      fprintf(out, "Image Palette:\n");

      for(i = 0; i < image.palette.count; ++i){
      fprintf(out, "%ld:", i);
      writeColor(image, *(Color *)image.palette.list[i], out);
      fprintf(out, "\n");
      }

      fprintf(out, "\n");

      }*/
}

Transparency * loadTransparency(ImageHeader header, DataStream stream)
{
    Transparency * transparency;
    size_t i;

    transparency = malloc(sizeof(Transparency));

    if(header.colorType == GREYSCALE_COLOR)
        transparency->color.greyscale = read16BitsNumber(&stream);
    else if(header.colorType == TRUECOLOR_COLOR){
        /* TODO: Proper order? */
        transparency->color.rgb.R = read16BitsNumber(&stream);
        transparency->color.rgb.B = read16BitsNumber(&stream);
        transparency->color.rgb.G = read16BitsNumber(&stream);
    } else if(header.colorType == INDEXED_COLOR){


        transparency->transparentIndices = getNewDataList(NULL, NULL);

        for(i = 0; i < stream.list.count; ++i){
            addByteToDataList(
                &transparency->transparentIndices,
                readStreamByte(&stream)
                );
        }

    }

    return transparency;
}

SignificantBits * loadSignificantBits(ImageHeader header, DataStream stream)
{
    SignificantBits * significantBits;

    significantBits = malloc(sizeof(SignificantBits));

    if(header.colorType == GREYSCALE_COLOR)
        significantBits->significantGreyscaleBits = readStreamByte(&stream);
    else if(header.colorType == TRUECOLOR_COLOR ||
            header.colorType == INDEXED_COLOR){

        significantBits->significantRedBits = readStreamByte(&stream);
        significantBits->significantGreenBits = readStreamByte(&stream);
        significantBits->significantBlueBits = readStreamByte(&stream);

    } else if(header.colorType == GREYSCALE_ALPHA_COLOR){

        significantBits->significantGreyscaleBits = readStreamByte(&stream);
        significantBits->significantAlphaBits = readStreamByte(&stream);
    } else if(header.colorType == TRUECOLOR_ALPHA_COLOR){

        significantBits->significantRedBits = readStreamByte(&stream);
        significantBits->significantGreenBits = readStreamByte(&stream);
        significantBits->significantBlueBits = readStreamByte(&stream);
        significantBits->significantAlphaBits = readStreamByte(&stream);
    }

    return significantBits;
}

void writeSignificantBits(
    SignificantBits * significantBits,
    ImageHeader header,
    FILE * out)
{
    if(significantBits != NULL){

        fprintf(out, "Significant bits\n");

        if(header.colorType == GREYSCALE_COLOR)

	    fprintf(out, "significant greyscale bits: %d\n",
		    significantBits->significantGreyscaleBits);

        else if(header.colorType == TRUECOLOR_COLOR ||
                header.colorType == INDEXED_COLOR){

	    fprintf(out, "significant red bits: %d\n",
		    significantBits->significantRedBits);
	    fprintf(out, "significant green bits: %d\n",
		    significantBits->significantGreenBits);
	    fprintf(out, "significant blue bits: %d\n",
		    significantBits->significantBlueBits);


        } else if(header.colorType == GREYSCALE_ALPHA_COLOR){

	    fprintf(out, "significant greyscale bits: %d\n",
		    significantBits->significantGreyscaleBits);

	    fprintf(out, "significant alpha bits: %d\n",
		    significantBits->significantAlphaBits);


        } else if(header.colorType == TRUECOLOR_ALPHA_COLOR){
	    fprintf(out, "significant red bits: %d\n",
		    significantBits->significantRedBits);
	    fprintf(out, "significant green bits: %d\n",
		    significantBits->significantGreenBits);
	    fprintf(out, "significant blue bits: %d\n",
		    significantBits->significantBlueBits);
	    fprintf(out, "significant alpha bits: %d\n",
		    significantBits->significantAlphaBits);
        }
    }
}



DataList uninterlace(DataList data, ImageHeader header)
{
    DataList uninterlaced;
    size_t col,row;
    int pass;
    size_t i;

    int startingRow[7]  = { 0, 0, 4, 0, 2, 0, 1 };
    int startingCol[7]  = { 0, 4, 0, 2, 0, 1, 0 };
    int rowIncrement[7] = { 8, 8, 8, 4, 4, 2, 2 };
    int colIncrement[7] = { 8, 8, 4, 4, 2, 2, 1 };

    uninterlaced = getNewDataList(NULL, NULL);

    /* TODO: clean up.*/
    uninterlaced = allocateNewDataList(
        header.width * header.height,
        header.width * header.height,
        NULL,
        NULL);

    i = 0;

    for(pass = 0; pass < 7; ++pass){

        row = startingRow[pass];

        while(row < header.height){

            col = startingCol[pass];

            while(col < header.width){
                uninterlaced.list[row * header.width + col] = data.list[i++];
                col += colIncrement[pass];

                printf("i:%ld\n", i);
            }

            row += rowIncrement[pass];

        }
    }

    return uninterlaced;
}
