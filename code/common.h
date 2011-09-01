#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>

void assertFile(FILE * fp,char * errorStr);
void assertFileOpened(FILE * fp);

/* change the file extension of a file name.
   If it didn't have a extension to begin with it is added.
   Do remember to the free allocated for the returned string.*/
char * changeExtension(char * fileName,char * newExtension);

#endif /* _COMMON_H_ */
