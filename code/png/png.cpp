#include "png.h"
#include <string.h>
#include <cctype>
#include <cstdlib>
#include <cmath>

using std::vector;

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
        delete image.renderingIntent;

    if(image.imageGamma != NULL)
        delete image.imageGamma;

    if(image.timeStamp != NULL)
        delete image.timeStamp;

    if(image.backgroundColor != NULL)
        delete image.backgroundColor;

    if(image.pixelDimensions != NULL)
        delete image.pixelDimensions;

    if(image.significantBits != NULL)
        delete image.significantBits;

    if(image.chromaticities != NULL)
        delete image.chromaticities;

    if(image.imageHistogram != NULL){
        delete image.imageHistogram;
    }

    if (image.suggestedPalettes != NULL)
	delete image.suggestedPalettes;

    if(image.transparency != NULL)
        delete image.transparency;

    if(image.palette != NULL)
        delete image.palette;

    for(size_t i = 0; i < image.textDataList.size(); ++i)
	    delete [] image.textDataList[i].str;

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
    image.suggestedPalettes = NULL;

    return image;
}

PNG_Image loadPNG(FILE * in)
{
    PNG_Image image;
    Chunk chunk;

    int idatFound;

    image = getEmptyPNG_Image();

    loadSignature(image.signature, in);
    image.header = loadImageHeader(in);

    chunk = loadChunk(in);

    idatFound = 0;

    /* Load the chunks in the file consecutively until the IEND chunk is found*/
    while(!isChunkType(chunk, IEND)){

	DataStream stream(chunk.data,PNG_ENDIAN);

        if(isChunkType(chunk, PLTE)){
            image.palette = loadPalette(stream);
        }

        else if(isChunkType(chunk, IDAT)){

	    /* Load all the consecutive IDAT chunks until a non IDAT chunk is found. */

            vector<BYTE> encodedColorData;

	    encodedColorData.insert(
		encodedColorData.end(),
		chunk.data.begin(),
		chunk.data.end());

            chunk = loadChunk(in);

            idatFound = 1;

            while(isChunkType(chunk, IDAT)){

	    encodedColorData.insert(
		encodedColorData.end(),
		chunk.data.begin(),
		chunk.data.end());

                chunk = loadChunk(in);
            }

            image.colorData = loadColorData(encodedColorData, image.header);

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
	    image.textDataList.push_back(loadTextualData(stream, 0, chunk.length));

        else if(isChunkType(chunk, zTXt))
	    image.textDataList.push_back(loadTextualData(stream, 1, chunk.length));

        else if(isChunkType(chunk, sBIT))
            image.significantBits = loadSignificantBits(image.header, stream);

        else if(isChunkType(chunk, cHRM))
            image.chromaticities = loadPrimaryChromaticities(stream);

        else if(isChunkType(chunk, hIST))
            image.imageHistogram = loadImageHistogram(stream);

        else {
            if(!isCriticalChunk(chunk)){
                printWarning("Unknown ancillary chunk %s found, skipping chunk.\n",
                             chunk.type);
            } else{
                printError("Unknown critcal chunk %s found.\n", chunk.type);
                exit(1);
            }
        }

        if(idatFound){
            chunk = chunk;
            idatFound = 0;
        } else{
            chunk = loadChunk(in);
        }
    }
    return image;
}

int isChunkType(Chunk chunk, const char * chunkType)
{
    return (!strcmp(chunk.type, chunkType));
}

void writePNG(PNG_Image image, FILE * out)
{
    writeSignature(image.signature, out);
    writeHeader(image.header, out);
    writePalette(image, out);

    writeRenderingIntent(image.renderingIntent, out);
    writeImageGamma(image.imageGamma, out);
    writePixelDimensions(image.pixelDimensions, out);
    writeTimeStamp(image.timeStamp, out);
    writeBackgroundColor(image, image.backgroundColor, out);
    writeSignificantBits(image.significantBits, image.header, out);
    writePrimaryChromaticities(image.chromaticities, out);
    writeImageHistogram(image.imageHistogram, out);

    writeTextDataList(image.textDataList, out);

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

    for(INT32 i = 0; i < chunk.length; ++i)
	chunk.data.push_back(getc(in));

    fread(&chunk.CRC, sizeof(INT32), 1, in);
    chunk.CRC = htonl(chunk.CRC);
    verbosePrint("Chunk CRC: %ld\n", chunk.CRC);

    validateCRC(chunk);

    return chunk;
}

void validateCRC(Chunk chunk)
{
    vector<BYTE> checkData;

    INT32 calcCRC;
    size_t i;

    /* Calculate the CRC of the chunk type field and data fields*/

    for(i = 0; i < 4; ++i){
        BYTE b = chunk.type[i];
	checkData.push_back(b);
    }

    checkData.insert(checkData.end(), chunk.data.begin(), chunk.data.end());

    calcCRC = crc32(checkData);

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

    headerChunk = loadChunk(in);

    DataStream stream(headerChunk.data, PNG_ENDIAN);
    header.width = stream.read32BitsNumber();
    header.height = stream.read32BitsNumber();
    header.bitDepth = stream.readStreamByte();
    header.colorType = stream.readStreamByte();
    header.compressionMethod = stream.readStreamByte();
    header.filterMethod = stream.readStreamByte();
    header.interlaceMethod = stream.readStreamByte();

    return header;
}

BYTE * loadRenderingIntent(DataStream & stream)
{
    BYTE * b;

    b = new BYTE;

    *b = stream.readStreamByte();

    return b;
}

#define CRC32_POLY 0xEDB88320

unsigned int crc32(vector<BYTE> data){

    unsigned long reminder = 0xFFFFFFFF; /* standard initial value in CRC32 */
    unsigned long i;
    unsigned long bit;
    BYTE b;

    for(i = 0; i < data.size(); ++i){
        b = data[i];
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

    gamma = new INT32;

    *gamma = stream.read32BitsNumber();

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

    pixelDimensions = new PixelDimensions;

    pixelDimensions->x = stream.read32BitsNumber();
    pixelDimensions->y = stream.read32BitsNumber();

    pixelDimensions->unitSpecifier = stream.readStreamByte();

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

    timeStamp = new TimeStamp;

    timeStamp->year = stream.read16BitsNumber();

    timeStamp->month = stream.readStreamByte();
    timeStamp->day = stream.readStreamByte();
    timeStamp->hour = stream.readStreamByte();
    timeStamp->minute = stream.readStreamByte();
    timeStamp->second = stream.readStreamByte();

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
/* bgbn4a08.png*/
Color * loadBackgroundColor(ImageHeader header, DataStream stream)
{
    Color * backgroundColor;

    backgroundColor = new Color;

    if(header.colorType == GREYSCALE_COLOR){

        backgroundColor->greyscale = stream.read16BitsNumber();

    } else if(header.colorType == GREYSCALE_ALPHA_COLOR){

        backgroundColor->greyscaleAlpha.greyscale = stream.read16BitsNumber();

        /* TODO: does this work? */
        backgroundColor->greyscaleAlpha.alpha =
            getMaximumChannelValue(header);
    } else if(header.colorType == TRUECOLOR_ALPHA_COLOR){

        backgroundColor->rgba.R = stream.read16BitsNumber();
        backgroundColor->rgba.G = stream.read16BitsNumber();
        backgroundColor->rgba.B = stream.read16BitsNumber();

        backgroundColor->rgba.A =
            getMaximumChannelValue(header);
    } else if(header.colorType == TRUECOLOR_COLOR){

        backgroundColor->rgb.R = stream.read16BitsNumber();
        backgroundColor->rgb.G = stream.read16BitsNumber();
        backgroundColor->rgb.B = stream.read16BitsNumber();
    } else if(header.colorType == INDEXED_COLOR){

        backgroundColor->index = stream.readStreamByte();
    }

    return backgroundColor;
}

/* ch1n3p04 */
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
        writeColor(image,*backgroundColor, out,1);
        fprintf(out,"\n");
    }
}

void writeColor(PNG_Image image,
                Color color,
                FILE * out,
                int backgroundColor)
{
    Color indexedColor;
    BYTE alpha;

    if(image.header.colorType == GREYSCALE_COLOR){

        if(!backgroundColor && image.transparency != NULL){

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

        if(!backgroundColor && image.transparency != NULL &&
           color.rgb.R == image.transparency->color.rgb.R &&
           color.rgb.G == image.transparency->color.rgb.G &&
           color.rgb.B == image.transparency->color.rgb.B)
            fprintf(out, "(transparent)");
        else
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

        indexedColor = (*image.palette)[color.index];

        if(!backgroundColor && image.transparency != NULL &&
           color.index < image.transparency->transparentIndices.size()){

            /* If the color is not there, it is 255 by default? check png spec*/
            alpha = image.transparency->transparentIndices[color.index];

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

TextualData loadTextualData(
    DataStream stream,
    int compressed,
    INT32 chunkLength)
{
    /*img/ctzn0g04.png */

    TextualData data;
    char ch;
    size_t i;
    size_t textLength;
    vector<BYTE> compressedData, decompressedData;
    int former;
    BYTE b;

    data.str = NULL;

    /* Read the keyword. */
    i = 0;
    do{
        ch = stream.readStreamByte();

        data.keyword[i++] = ch;

    } while(ch != '\0');


    if(compressed){

        /* Uncompress the data.*/

        /* read the compression method(there is only one, so this value is skipped). */
        stream.readStreamByte();
	++i;

	/* TODO: does this work?*/
        for(; i < stream.getStreamSize(); ++i)
	    compressedData.push_back(stream.readStreamByte());

        former = verbose;
        verbose = 0;
        decompressedData = ZLIB_Decompress(compressedData);
        verbose = former;

        data.str = new char[decompressedData.size() + 1];

        for(i = 0; i < decompressedData.size(); ++i){
            b = decompressedData[i];
            data.str[i] = (char)b;
        }
        data.str[i] = '\0';
    } else{
        /* Simply read the data.

         */

        textLength = chunkLength - i;

        /* +1 makes space for the null character. */
        data.str = new char[textLength + 1];

        for(i = 0; i < (textLength); ++i){
            data.str[i] = (char)stream.readStreamByte();
        }

        data.str[i] = '\0';
    }

    return data;
}

void writeTextDataList(vector<TextualData> textDataList, FILE * out)
{
    size_t i;
    if(textDataList.size() > 0){

        for(i = 0; i < textDataList.size(); ++i){
            fprintf(out, "Textual data %ld:\n", i+1);
            fprintf(out, "Keyword: %s\n", textDataList[i].keyword);
            fprintf(out, "Text: %s\n", textDataList[i].str);
        }
    }
}

std::vector<Color> loadColorData(vector<BYTE> data, ImageHeader header)
{
    int former;
    vector<BYTE> decompressed, unfiltered;
    vector<Color> colorData;
    former = verbose;
    verbose = 0;
    decompressed = ZLIB_Decompress(data);
    verbose = former;

/*    printf("uncompressed:\n");
    for(size_t i = 0; i < decompressed.size(); ++i){
	printf("%d\n", decompressed[i]);
    } */

    /* Unfilter. */

    unfiltered = unfilterImage(decompressed, header);
/*    printf("unfiltered:\n");
    for(size_t i = 0; i < unfiltered.size(); ++i){
	printf("%d\n", unfiltered[i]);
    } */

    /* Undo the Scanline serialization; that is, split up the bytes into colors. */

    colorData = splitUpColorData(unfiltered, header);

    if(header.interlaceMethod == ADAM7_INTERLACE){
        return uninterlace(colorData, header);
    } else
        return colorData;
}

void writeColorData(PNG_Image image, FILE * out)
{
    size_t row,col;

    fprintf(out,"Color Data:\n");

    for(row = 0; row < image.header.height; ++row){

        for(col = 0; col < image.header.width; ++col){

            writeColor(image,image.colorData[row * image.header.width + col], out, 0);
            fprintf(out,",");
        }

        fprintf(out,"\n\n");
    }
}

vector<BYTE> unfilterSubImage(
    DataStream & stream,
    size_t imageWidth,
    size_t height,
    const ColorInfo & info)
{
    vector<BYTE> unfiltered;
    size_t scanline;
    int filterType;
    size_t i;

    size_t width;

    BYTE unfilteredByte;

    /* The byte being filtered. */
    BYTE x;

    /* The previous pixel channel pixel value. */
    BYTE a;

    /* The upper byte in the previous scanline. */
    BYTE b;

    BYTE c;

    size_t bpp;

    bpp =  (int)ceil((double)(info.numChannels * info.channelBitDepth) / 8.0);

    width = ceil((info.channelBitDepth *  imageWidth * info.numChannels) / 8.0);

    /* Read scanline after scanline*/
    for(scanline = 0; scanline < height; ++scanline){

        /* Read the filter type. */
        filterType = stream.readStreamByte();

        for(i = 0; i < width; ++i){

            x = stream.readStreamByte();

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

                unfilteredByte = (BYTE)((unsigned int)x +

                                        floor(((unsigned int)a + (unsigned int)b) / 2)
                    )% 256;

            } else if(filterType == PAETH_FILTER){

                a = compute_a(i, bpp, unfiltered);
                b = compute_b(scanline, width, unfiltered);
                c = compute_c(i, bpp, scanline, width, unfiltered);

                unfilteredByte = (BYTE)((unsigned int)x +
                                        paethPredictor(a,b,c)) % 256;

            }

	    unfiltered.push_back(unfilteredByte);
        }
    }

    return unfiltered;

}

vector<BYTE> unfilterImage(vector<BYTE> data, ImageHeader header)
{
    size_t pass;
    size_t passes;

    ColorInfo info;

    InterlacedSubImagesSizes sizes;

    vector<BYTE> subimage;
    vector<BYTE> fullImage;

    if(header.interlaceMethod == ADAM7_INTERLACE){
	passes = 7;
	sizes = calcInterlacedSubImagesSizes(header.width, header.height);
    } else {
	passes = 1;
	sizes = calcImageSize(header.width, header.height);
    }

    info = getColorInfo(header);

    DataStream stream(data, PNG_ENDIAN);

    for(pass = 0; pass < passes; ++pass){

        if(sizes.sizes[pass].width != 0 && sizes.sizes[pass].height != 0){

            subimage = unfilterSubImage(
                stream,
                sizes.sizes[pass].width,
                sizes.sizes[pass].height,
                info);

	    fullImage.insert(fullImage.end(), subimage.begin(), subimage.end());
        }
    }

    return fullImage;
}

InterlacedSubImagesSizes calcImageSize(size_t width, size_t height)
{
    InterlacedSubImagesSizes ret;

    ret.sizes[0].width = width;
    ret.sizes[0].height = height;

    return ret;
}

InterlacedSubImagesSizes calcInterlacedSubImagesSizes(size_t width, size_t height)
{
    InterlacedSubImagesSizes ret;

    int startingRow[7]  = { 0, 0, 4, 0, 2, 0, 1 };
    int startingCol[7]  = { 0, 4, 0, 2, 0, 1, 0 };
    int rowIncrement[7] = { 8, 8, 8, 4, 4, 2, 2 };
    int colIncrement[7] = { 8, 8, 4, 4, 2, 2, 1 };

    size_t  pass;
    size_t col;
    size_t  row;

    for(pass = 0; pass < 7; ++pass){

        ret.sizes[pass].width = 0;
        col = startingCol[pass];

        while(col < width){
            ++ret.sizes[pass].width;
            col += colIncrement[pass];
        }

        ret.sizes[pass].height = 0;
        row = startingRow[pass];

        while(row < height){
            ++ret.sizes[pass].height;
            row += rowIncrement[pass];
        }
    }

    return ret;
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

BYTE compute_a(size_t i, size_t bpp, const vector<BYTE> & unfiltered)
{
    if(i >= bpp)
        return unfiltered[unfiltered.size() - bpp];
    else
        return 0;
}

BYTE compute_b(size_t scanline, size_t width, const std::vector<BYTE> & unfiltered)
{
    if(scanline == 0)
        return 0;
    else
        return unfiltered[unfiltered.size() - width];
}

BYTE compute_c(
    size_t i,
    size_t bpp,
    size_t scanline,
    size_t width,
    const std::vector<BYTE> & unfiltered)
{
    if(scanline == 0)
        return 0;
    else {
        if(i >= bpp)
            return unfiltered[unfiltered.size() - width - bpp];
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

std::vector<Color> splitUpColorDataSubImage(
    DataStream & colorStream,
    BitReader & inBits,
    const ImageHeader & header,
    size_t width,
    size_t height,
    size_t dataCount
    )
{
    size_t i;
    vector<Color> colorData;
    Color color;

/*    printf("size:%ld\n", (width * height)); */


    for(i = 0; i < (width * height); ++i){

/*	printf("i:%ld\n", i); */

        if(header.colorType == GREYSCALE_COLOR){
            color.greyscale = readNextChannel(colorStream, inBits, header);
        }

        else if(header.colorType == INDEXED_COLOR){

            color.index = readNextChannel(colorStream, inBits, header);
        }

        else if(header.colorType == TRUECOLOR_COLOR){
            color.rgb.R = readNextChannel(colorStream, inBits, header);
            color.rgb.G = readNextChannel(colorStream, inBits, header);
            color.rgb.B = readNextChannel(colorStream, inBits, header);
        }

        else if(header.colorType == GREYSCALE_ALPHA_COLOR){

            color.greyscaleAlpha.greyscale = readNextChannel(colorStream, inBits, header);
            color.greyscaleAlpha.alpha = readNextChannel(colorStream, inBits, header);
        }

        else if(header.colorType == TRUECOLOR_ALPHA_COLOR){

            color.rgba.R = readNextChannel(colorStream, inBits, header);
            color.rgba.G = readNextChannel(colorStream, inBits, header);
            color.rgba.B = readNextChannel(colorStream, inBits, header);
            color.rgba.A = readNextChannel(colorStream, inBits, header);
        }

	colorData.push_back(color);

        if((i+1) % width == 0 &&
	   inBits.getPosition() != dataCount &&
	   header.bitDepth < 8){
/*	    printf("next\n"); */

	    inBits.readNextByte();
	    /* read next byte in stream*/
        }
    }

    return colorData;
}

vector<Color> splitUpColorData(vector<BYTE> data, ImageHeader header)
{
    size_t pass;
    size_t passes;

    vector<Color> subimage;
    vector<Color> fullImage;

    InterlacedSubImagesSizes sizes;

    if(header.interlaceMethod == ADAM7_INTERLACE){
	passes = 7;
	sizes = calcInterlacedSubImagesSizes(header.width, header.height);
    } else {
	passes = 1;
	sizes = calcImageSize(header.width, header.height);
    }

    DataStream colorStream(data,PNG_ENDIAN);
    BitIterReader inBits(data.begin(),MSBF);

    for(pass = 0; pass < passes; ++pass){

        if(sizes.sizes[pass].width != 0 && sizes.sizes[pass].height != 0){

            subimage =
                splitUpColorDataSubImage(colorStream, inBits, header,
                                         sizes.sizes[pass].width,
                                         sizes.sizes[pass].height,
					 data.size());
	    fullImage.insert(
		fullImage.end(),
		subimage.begin(),
		subimage.end());

        }
    }

    return fullImage;
}

unsigned long readNextChannel(
    DataStream & stream,
    BitReader & inBits,
    const ImageHeader & header)
{
    unsigned long channel;

    if(header.bitDepth == 8)
        channel = stream.readStreamByte();
    else if(header.bitDepth == 16)
        channel = stream.read16BitsNumber();
    else if(header.bitDepth < 8)
        /* Handle bit depths 1, 2 and 4*/
        channel = inBits.readBits(header.bitDepth);
/*	printf("channel:%ld\n", channel); */


    return channel;
}

vector<Color> * loadPalette(DataStream & stream)
{
    vector<Color> * palette;
    size_t size;
    size_t i;
    Color color;

    palette = new vector<Color>;

    size = stream.getStreamSize() / 3;

    for(i = 0; i < size; ++i){
        color.rgb.R = stream.readStreamByte();
        color.rgb.G = stream.readStreamByte();
        color.rgb.B = stream.readStreamByte();

        palette->push_back(color);
    }

    return palette;
}

void writePalette(PNG_Image image,FILE * out)
{
    size_t i;
    Color c;

    if(image.palette != NULL){
        fprintf(out, "Image Palette:\n");

        for(i = 0; i < image.palette->size(); ++i){

            c = (*image.palette)[i];

            fprintf(out, "%ld: (%d,%d,%d)\n", i, c.rgb.R, c.rgb.G , c.rgb.B);
        }

        fprintf(out, "\n");

    }
}

Transparency * loadTransparency(const ImageHeader & header, DataStream & stream)
{
    Transparency * transparency;
    size_t i;

    transparency = new Transparency;

    if(header.colorType == GREYSCALE_COLOR)
        transparency->color.greyscale = stream.read16BitsNumber();
    else if(header.colorType == TRUECOLOR_COLOR){
        /* TODO: Proper order? */
        transparency->color.rgb.R = stream.read16BitsNumber();
        transparency->color.rgb.B = stream.read16BitsNumber();
        transparency->color.rgb.G = stream.read16BitsNumber();
    } else if(header.colorType == INDEXED_COLOR){
        for(i = 0; i < stream.getStreamSize(); ++i){
	    transparency->transparentIndices.push_back(stream.readStreamByte());
        }

    }

    return transparency;
}

SignificantBits * loadSignificantBits(ImageHeader header, DataStream stream)
{
    SignificantBits * significantBits;

    significantBits = new SignificantBits;

    if(header.colorType == GREYSCALE_COLOR)
        significantBits->significantGreyscaleBits = stream.readStreamByte();
    else if(header.colorType == TRUECOLOR_COLOR ||
            header.colorType == INDEXED_COLOR){

        significantBits->significantRedBits = stream.readStreamByte();
        significantBits->significantGreenBits = stream.readStreamByte();
        significantBits->significantBlueBits = stream.readStreamByte();

    } else if(header.colorType == GREYSCALE_ALPHA_COLOR){

        significantBits->significantGreyscaleBits = stream.readStreamByte();
        significantBits->significantAlphaBits = stream.readStreamByte();
    } else if(header.colorType == TRUECOLOR_ALPHA_COLOR){

        significantBits->significantRedBits = stream.readStreamByte();
        significantBits->significantGreenBits = stream.readStreamByte();
        significantBits->significantBlueBits = stream.readStreamByte();
        significantBits->significantAlphaBits = stream.readStreamByte();
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

PrimaryChromaticities * loadPrimaryChromaticities(DataStream stream)
{
    PrimaryChromaticities * primaryChromaticities;

    primaryChromaticities = new PrimaryChromaticities;


    primaryChromaticities->whitePointX = stream.read32BitsNumber();
    primaryChromaticities->whitePointY = stream.read32BitsNumber();

    primaryChromaticities->RedX = stream.read32BitsNumber();
    primaryChromaticities->RedY = stream.read32BitsNumber();

    primaryChromaticities->GreenX = stream.read32BitsNumber();
    primaryChromaticities->GreenY = stream.read32BitsNumber();

    primaryChromaticities->BlueX = stream.read32BitsNumber();
    primaryChromaticities->BlueY = stream.read32BitsNumber();

    return primaryChromaticities;
}

void writePrimaryChromaticities(
    PrimaryChromaticities * primaryChromaticities,
    FILE * out)
{
    if(primaryChromaticities != NULL){

        fprintf(out, "Primary chromaticities and white point:\n");

        fprintf(out, "White point x:%0.4f\n",
                primaryChromaticities->whitePointX / 100000.0);
        fprintf(out, "White point y:%0.4f\n",
                primaryChromaticities->whitePointY / 100000.0);

        fprintf(out, "Red x:%0.4f\n",
                primaryChromaticities->RedX / 100000.0);
        fprintf(out, "Red y:%0.4f\n",
                primaryChromaticities->RedY / 100000.0);

        fprintf(out, "Green x:%0.4f\n",
                primaryChromaticities->GreenX / 100000.0);
        fprintf(out, "Green y:%0.4f\n",
                primaryChromaticities->GreenY / 100000.0);

        fprintf(out, "Blue x:%0.4f\n",
                primaryChromaticities->BlueX / 100000.0);
        fprintf(out, "Blue y:%0.4f\n",
                primaryChromaticities->BlueY / 100000.0);
    }
}

vector<INT16> * loadImageHistogram(DataStream & stream)
{
    vector<INT16> * histogram;
    size_t i;

    histogram = new vector<INT16>;

    for(i = 0; i < (stream.getStreamSize() / 2); ++i)
	histogram->push_back(stream.read16BitsNumber());

    return histogram;
}

void writeImageHistogram(vector<INT16> * imageHistogram, FILE * out)
{
    if(imageHistogram != NULL){

        fprintf(out, "Image histogram: \n");

        for(size_t i = 0; i < imageHistogram->size(); ++i)
            fprintf(out, "%d, ", (*imageHistogram)[i]);

        fprintf(out, "\n");
    }
}

vector<Color> uninterlace(const vector<Color> & data, const ImageHeader & header)
{
    size_t col,row;
    int pass;
    size_t i;

    int startingRow[7]  = { 0, 0, 4, 0, 2, 0, 1 };
    int startingCol[7]  = { 0, 4, 0, 2, 0, 1, 0 };
    int rowIncrement[7] = { 8, 8, 8, 4, 4, 2, 2 };
    int colIncrement[7] = { 8, 8, 4, 4, 2, 2, 1 };

    vector<Color> uninterlaced(header.width * header.height);

    i = 0;

    for(pass = 0; pass < 7; ++pass){

        row = startingRow[pass];

        while(row < header.height){

            col = startingCol[pass];

            while(col < header.width){

                uninterlaced[row * header.width + col] = data[i++];

                col += colIncrement[pass];

            }

            row += rowIncrement[pass];

        }
    }

    return uninterlaced;
}
