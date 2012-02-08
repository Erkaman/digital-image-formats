#ifndef _IO_H_
#define _IO_H_

#include "defs.h"
#include <cstdio>

extern int verbose;

void readStr(FILE * in,size_t length,char * str);

void assertFile(FILE * fp,const char * errorStr);

void assertFileOpened(FILE * fp);

size_t getFileSize(FILE * fp);    

void verbosePrint(const char * format, ...);
void printWarning(const char * format, ...);
void printError(const char * format, ...);

#endif /* _IO_H_ */
