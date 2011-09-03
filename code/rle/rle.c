#include "../common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void decode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;
    char c,length;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    length = -1;
    c = getc(in);
    while (c != EOF){
        if(length == -1)
            length = c;
        else{
            for(; length > 0; --length)
                putc(c,out);

            length = -1;
        }
        c = getc(in);
    }

    fclose(in);
    fclose(out);
}

void encode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;

    char c2,c1;
    char length;
    int passedFirstCharacter;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    length = 1;
    passedFirstCharacter = 0;

    c2 = getc(in);
    while (c2 != EOF){
        /* if it's not the first character. */
        if(passedFirstCharacter){
            if(c2 == c1 && length < CHAR_MAX)
                ++length;
            else{
                putc(length,out);
                putc(c1,out);
                length = 1;
            }
        }
	passedFirstCharacter = 1;
        c1 = c2;
        c2 = getc(in);
    }

    /* write the last bytes. */
    if(passedFirstCharacter){
        putc(length,out);
        putc(c1,out);
    }

    fclose(in);
    fclose(out);
}

int main(int argc, char *argv[])
{
    char * result;
    int decompress = 0;
    int n;

    if(argc == 1){
        printf("No arguments specified\n");
        return 1;
    }

    /* parse command line arguments */

    ++argv;
    n = 1;

    while(n < argc && (*argv)[0] == '-'){
        switch((*argv)[1]){
        case 'd':
            decompress = 1;
            break;
        }
        ++argv;
        ++n;
    }

    if((argc - n) < 2){
        printf("An input AND an output file must be specified");
        return 1;
    }

    if(decompress){
        decode(argv[0],argv[1]);
    }else{
        encode(argv[0],argv[1]);
    }

    free(result);

    return 0;
}
