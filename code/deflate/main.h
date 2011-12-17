#ifndef _MAIN_H_
#define _MAIN_H_

#include "zlib.h"
#include "../data_list.h"
#include "../io.h"

void printHelp(void);

void ZLIB_DecompressFile(FILE * in, FILE * out);

DataList getCompressedData(FILE * in);

#endif /* _MAIN_H_ */

