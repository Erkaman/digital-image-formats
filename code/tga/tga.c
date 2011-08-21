#include <stdio.h>
#include <stdlib.h>

#include "tga.h"

void loadTGA(const char * file)
{
    FILE * fp;

    fp=fopen(file,"rb");

    if(fp == NULL){
	perror("An error occurred opening the file");
	exit(1);
    }

    
}

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
