#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tga.h"

int main(int argc, char *argv[])
{
    if(argc == 1){
	printf("No file was specified for loading");
	return 1;
    } else{
	loadTGA(argv[1]);
    }
    return 0;
}

void loadTGA(char * file)
{
    FILE * in;
    FILE * out;

    in = fopen(file,"rb");

    if(in == NULL){
	perror("An error occurred opening the file");
	exit(1);
    }

    TGAHeader tgah;

    fread(&tgah,sizeof(TGAHeader),1,in);

    /* Works because all tga files has a tga extension(I hope).*/
    strip_extension(file);
    strcat(file,".dmp");

    out = fopen(file,"wb");

    fprintf(out,"Id Length:%d\n",tgah.IDLength);
    fprintf(out,"Color map type:%d\n",tgah.colorMapType);
    fprintf(out,"Image type:%d\n",tgah.imageType);
    fprintf(out,"Color map type:%d\n",tgah.colorMapStart);
    fprintf(out,"Color map length:%d\n",tgah.colorMapLength);
    fprintf(out,"Color map depth:%d\n",tgah.colorMapDepth);
    fprintf(out,"X origin:%d\n",tgah.xOrigin);
    fprintf(out,"Y origin:%d\n",tgah.yOrigin);
    fprintf(out,"Width:%d\n",tgah.width);
    fprintf(out,"Height:%d\n",tgah.height);
    fprintf(out,"pixelDepth:%d\n",tgah.pixelDepth);
    fprintf(out,"Image Descriptor:%d\n",tgah.imageDescriptor);

}

void strip_extension(char * fileName)
{
    char * extBeg;
    int n;
    extBeg = strrchr(fileName,'.');
    if(extBeg == NULL);
	//return extBeg;
    else{
	n = extBeg - fileName;
	fileName[n] = '\0';
    }
}
