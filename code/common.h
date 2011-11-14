#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdint.h>

#define BYTE uint8_t
#define BYTE_MAX UINT8_MAX

BYTE readByte(FILE * fp);
void writeByte(BYTE b,FILE * fp);
void readStr(FILE * fp,size_t length,char * str);

void assertFile(FILE * fp,char * errorStr);
void assertFileOpened(FILE * fp);

BYTE firstNBits(BYTE num,unsigned int n);
BYTE lastNBits(BYTE num,unsigned int n);

/* change the file extension of a file name.
   If it didn't have a extension to begin with it is added.
   Do remember to the free allocated for the returned string.*/
char * changeExtension(char * fileName,char * newExtension);

char * strAppend ( char * str, const char * appendee );
char * strncpyBack( char * destination, const char * source, size_t num );


#endif /* _COMMON_H_ */
