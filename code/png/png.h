#ifndef _PNG_H_
#define _PNG_H_

#include "../common.h"

typedef struct {
    DataContainer signatureData;
} PNG_Signature;

typedef struct {
    PNG_Signature signature;
} PNG_Image;

void dumpPNG(FILE * in, FILE * out);

#endif /* _PNG_H_ */
