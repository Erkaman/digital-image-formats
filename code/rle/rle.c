#include <stdio.h>
#include <stdlib.h>
/* TODO: organize common functions and utilities into headers and files. */
void assertFileP(FILE * fp)
{
    if(fp == NULL){
        perror("An error occurred opening the file");
        exit(1);
    }
}


void encode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;
    char c,formerC;
    char counter;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileP(in);
    assertFileP(out);

    counter = 0;
    formerC = -1;
    while ((c = getc(in)) != EOF){
        /* if it's not the first character. */
        if(formerC != -1){
            if(c == formerC){
                ++formerC;
            }else{
                putc(counter,out);
                putc(formerC,out);
                counter = 0;
            }

            formerC = c;
        }else{
            formerC = c;
            ++formerC;
        }
    }

    fclose(in);
    fclose(out);
}

int main(int argc, char *argv[])
{
    if(argc != 1)
        encode(argv[1],"enc");

    return 0;
}
