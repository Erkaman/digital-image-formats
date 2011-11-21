#ifndef _MAIN_H_
#define _MAIN_H_

#include "../common.h"
#include "zlib.h"

void printHelp(void);

void ZLIB_DecompressFile(FILE * in, FILE * out);

DataContainer getCompressedData(FILE * in);

#endif /* _MAIN_H_ */

