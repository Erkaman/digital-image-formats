#ifndef _MAIN_H_
#define _MAIN_H_

#include "../io.h"
#include "../bitwise.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void printHelp(void);

/* compression */

void LZW_Compress(FILE * in,FILE * out);
void outputCode(unsigned int code,FILE * out);
int findMatch(unsigned int hash_prefix,unsigned int hash_character);

/* decompression */

void LZW_Decompress(FILE * in,FILE * out);
void translateCode(unsigned int newCode);
char printString(FILE * out);
unsigned int inputCode(FILE *input);

typedef struct {
    unsigned int stringCode;
    unsigned int characterCode;
} tableEntry;

#endif /* _MAIN_H_ */
