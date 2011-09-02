#include "../common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void decode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;
    char c,count;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    count = -1;
    while ((c = getc(in)) != EOF){
        /* if the counter hasn't been assigned to yet. */
        if(count == -1)
            count = c;
        else{
            for(; count > 0; --count)
                putc(c,out);

            count = -1;
        }
    }

    fclose(in);
    fclose(out);
}

/* FIXME: if a number is repeated more than the max value of char this
 * algorithm will break.   */


void encode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;
    char c,formerC;
    char counter;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    counter = 1;
    formerC = -1;

    while ((c = getc(in)) != EOF){
        /* if it's not the first character. */
        if(formerC != -1){
            if(c == formerC)
                ++counter;
            else{
                putc(counter,out);
                putc(formerC,out);
                counter = 1;
            }
        }
        formerC = c;
    }

    /* write the last bytes. */
    if(formerC != -1){
        putc(counter,out);
        putc(formerC,out);
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
