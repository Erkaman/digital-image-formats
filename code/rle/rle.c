#include "../common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void decode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;
    BYTE c,length;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    while (!feof(in)){
        length = readByte(in);
        c = readByte(in);

	/* if the end of the file was reached. */
	if(feof(in)){
	    break;
	}

        for(; length > 0; --length)
            writeByte(c,out);
    }

    fclose(in);
    fclose(out);
}

void encode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;

    BYTE c2,c1;
    BYTE length;
    int passedFirstCharacter;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    length = 1;
    passedFirstCharacter = 0;

    c2 = readByte(in);
    while (!feof(in)){
        /* if it's not the first character. */
        if(passedFirstCharacter){
            if(c2 == c1 && length < BYTE_MAX)
                ++length;
            else{
                writeByte(length,out);
                writeByte(c1,out);

                length = 1;
            }
        }
        passedFirstCharacter = 1;
        c1 = c2;
        c2 = readByte(in);
    }

    /* write the last bytes. */
    if(passedFirstCharacter){
        writeByte(length,out);
        writeByte(c1,out);
    }

    fclose(in);
    fclose(out);
}


void encode_opt(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;

    char c2,c1;
    char length;
    int passedFirstCharacter;
    char data[127];
    int packetType;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    length = 1;
    passedFirstCharacter = 0;

    packetType = 0;

    c2 = getc(in);
    while (c2 != EOF){
        /* if it's not the first character. */
        if(passedFirstCharacter){
            if(c2 == c1 && length < CHAR_MAX){
                packetType = 1;
                ++length;
            }
            else{
                if(packetType == 1){
                    /* write the packet of type 1 */

                }
                else{

                }
                putc(length,out);
                putc(c1,out);
                length = 1;
            }
        }
        passedFirstCharacter = 1;
        c1 = c2;
        c2 = getc(in);
    }

    /* write the last bytes.
       if(passedFirstCharacter){
       putc(length,out);
       putc(c1,out);
       }*/

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
