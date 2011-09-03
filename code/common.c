#include "common.h"
#include <stdlib.h>
#include <string.h>

void assertFile(FILE * fp,char * errorStr)
{
    if(fp == NULL){
        perror(errorStr);
        exit(1);
    }
}

void assertFileOpened(FILE * fp)
{
    assertFile(fp,"An error occurred opening the file");
}

char * changeExtension(char * fileName,char * newExtension)
{
    char * changedFileName;
    int size;
    char * extensionEnd;

    extensionEnd = strrchr(fileName,'.');

    if(extensionEnd == NULL)
        extensionEnd = fileName + strlen(fileName);

    size = (extensionEnd - fileName + 1) + strlen(newExtension);

    changedFileName = (char *) malloc(size);

    strncpy(changedFileName, fileName, extensionEnd - fileName);

    changedFileName[extensionEnd- fileName] = '.';
    changedFileName[extensionEnd- fileName + 1] = '\0';

    strcat(changedFileName, newExtension);

    return changedFileName;
}

BYTE readByte(FILE * fp)
{
    BYTE s;
    fread(&s,sizeof(BYTE),1,fp);
    return s;
}

void writeByte(BYTE b,FILE * fp)
{
    fwrite(&b,sizeof(unsigned char),1,fp);
}
