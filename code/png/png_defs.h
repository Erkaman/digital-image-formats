#ifndef _PNG_DEFS_H_
#define _PNG_DEFS_H_

/* Constants */

#include "../data_stream.h"
#include <stdint.h>

#define SIGNATURE_LENGTH 8


/* An unsigned 4 byte number.*/
#define INT32 uint32_t

/* An unsigned 2 byte number.*/
#define INT16 uint32_t

/* Header chunk. */
#define IHDR "IHDR"

/* Palette */
#define PLTE "PLTE"

/* Image data chunk. */
#define IDAT "IDAT"

/* Ending chunk. */
#define IEND "IEND"

/* Ending chunk. */
#define tRNS  "tRNS"

#define cHRM  "cHRM"

#define gAMA "gAMA"

#define iCCP "iCCP"

#define sBIT "sBIT"

#define sRGB  "sRGB"

#define tEXt "tEXt"

#define zTXt "zTXt"

#define iTXt "iTXt"

#define bKGD "bKGD"

#define hIST "hIST"

#define pHYs "pHYs"

#define sPLT "sPLT "

#define tIME "tIME"


#define GREYSCALE_COLOR 0
#define TRUECOLOR_COLOR 2
#define INDEXED_COLOR 3
#define GREYSCALE_ALPHA_COLOR 4
#define TRUECOLOR_ALPHA_COLOR 6

/* The only allowed compression method. */
#define DEFLATE_COMPRESSION_METHOD 0

/* The only allowed filter method. */
#define FILTER_METHOD_0 0

/* Filter types: */

#define NO_FILTER 0
#define SUB_FILTER 1
#define UP_FILTER 2
#define AVERAGE_FILTER 3
#define PAETH_FILTER 4

/* Interlacing types. */

#define NO_INTERLACE 0
#define ADAM7_INTERLACE 1

#define PERCEPTUAL_RENDERING_INTENT 0
#define RELATIVE_COLORIMETRIC_RENDERING_INTENT 1
#define SATURATION_RENDERING_INTENT 2
#define ABSOLUTE_COLORIMETRIC_RENDERING_INTENT 3

/* Units specifiers. Used in the pHYs chunk. */

#define UNKNOWN_UNIT 0
#define METRE_UNIT 1

#define PNG_ENDIAN ENDIAN_BIG

#endif /* _PNG_DEFS_H_ */
