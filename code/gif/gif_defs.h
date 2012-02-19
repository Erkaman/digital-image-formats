#ifndef _GIF_DEFS_H_
#define _GIF_DEFS_H_

/* The GIF standard refers to an unsigned 16-bit number as an
 * "unsigned". */
#define UNSIGNED unsigned short

#define FLAG int

/* the color depth of the colors in a GIF is always 24 */
#define COLOR_DEPTH 24
#define TRAILER 0x3b
#define EXTENSION_INTRODUCER 0x21
#define BLOCK_TERMINATOR 0x00

#define GRAPHIC_CONTROL_LABEL 0xf9
#define COMMENT_LABEL 0xfe
#define APPLICATION_EXTENSION_LABEL 0xff

#define PLAIN_TEXT_LABEL 0x01
#define IMAGE_SEPARATOR 0x2c

typedef struct{
    char signature[4];
    char version[4];
} GIFHeader;

typedef std::vector<BYTE>  GIFDataSubBlocks;

typedef struct{
    UNSIGNED logicalScreenWidth;
    UNSIGNED logicalScreenHeight;

    /* packed fields */
    FLAG globalColorTableFlag;
    BYTE colorResolution;
    FLAG sortFlag;
    BYTE globalColorTableSize;

    BYTE backgroundColorIndex;
    BYTE pixelAspectRatio;

} GIFLogicalScreenDescriptor;

typedef struct{
    BYTE imageSeparator;

    UNSIGNED imageLeftPosition;
    UNSIGNED imageTopPosition;

    UNSIGNED imageWidth;
    UNSIGNED imageHeight;

    /* packed fields */
    FLAG localColorTableFlag;
    FLAG interlaceFlag;
    FLAG sortFlag;
    BYTE reserved;
    BYTE localColorTableSize;

} GIFImageDescriptor;

typedef struct{
    BYTE extensionIntroducer;
    BYTE graphicControlLabel;
    BYTE blockSize;

    /* packed fields */
    BYTE reserved;
    BYTE disposalMethod;
    FLAG userInputFlag;
    FLAG transparencyFlag;

    UNSIGNED delayTime;
    BYTE transparencyIndex;
    BYTE blockTerminator;

} GIFGraphicControl;

typedef struct{
    BYTE extensionIntroducer;
    BYTE extensionLabel;

    BYTE blockSize;

    char applicationIdentifier[9];

    char applicationAuthenticationCode[4];

    GIFDataSubBlocks applicationData;

    BYTE blockTerminator;

} GIFApplicationExtension;

typedef struct{
    BYTE extensionIntroducer;
    BYTE commentLabel;

    char * commentData;

    BYTE blockTerminator;

} GIFCommentExtension;

typedef struct{

    BYTE extensionIntroducer;
    BYTE plainTextLabel;

    BYTE blockSize;

    UNSIGNED textGridLeftPosition;
    UNSIGNED textGridTopPosition;

    UNSIGNED textGridWidth;
    UNSIGNED textGridHeight;

    BYTE characterCellWidth;
    BYTE characterCellHeight;

    BYTE textForegroundColorIndex;
    BYTE textBackgroundColorIndex;

    char * plainTextData;

    BYTE blockTerminator;

} GIFPlainTextExtension;

typedef struct {
    BYTE r;
    BYTE g;
    BYTE b;
} GIFColor;

#endif /* _GIF_DEFS_H_ */
