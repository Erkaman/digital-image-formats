#include "../common.h"
#include <stdio.h>
#include <stdlib.h>

/*int findExtensionBeg(char * )*/

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
    if(argc != 1){

        encode(argv[1],"enc");
    }

    return 0;
}
