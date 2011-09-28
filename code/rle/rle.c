#include "../common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define RUN_LENGTH_PACKET 1
#define RAW_PACKET 0

void RLE_Encode(char * inputfile,char * outputfile);
void RLE_Decode(char * inputfile,char * outputfile);
void packBitsEncode(char * inputfile,char * outputfile);
void packBitsDecode(char * inputfile,char * outputfile);
void writeRunLengthPacket(BYTE length,BYTE data,FILE * fp);
void writeRawPacket(BYTE length,BYTE * data,FILE * fp);
void printHelp(void);

int main(int argc, char *argv[])
{
    int decompress = 0;
    int optimize = 0;
    int help = 0;
    int n;

    if(argc == 1){
        printf("No arguments specified.\n");
        printf("Try --help for more information.\n");
        return 1;
    }

    /* parse command line arguments */

    ++argv;
    n = 1;

    while(n < argc && (*argv)[0] == '-'){

        if(!strcmp(*argv,"--help"))
            help = 1;
        else
            switch((*argv)[1]){
            case 'd':
                decompress = 1;
                break;
            case 'p':
                optimize = 1;
                break;
            }
        ++argv;
        ++n;
    }

    if(help){
        printHelp();
        return 0;
    }

    if((argc - n) < 2){
        printf("An input AND an output file must be specified");
        return 1;
    }


    if(optimize){
        if(decompress){
            packBitsDecode(argv[0],argv[1]);
        } else{
            packBitsEncode(argv[0],argv[1]);
        }
    } else{
        if(decompress){
            RLE_Decode(argv[0],argv[1]);
        }else{
            RLE_Encode(argv[0],argv[1]);
        }
    }

    return 0;
}

void printHelp(void)
{
    printf("Usage: rle [OPTION]... IN OUT\n");
    printf("Compress or decompress the IN file to the OUT file using the Run Length Encoding algorithm.\n");

    printf("  --help\tDisplay this help message.\n");
    printf("  -p\tUse the packBits algorithm for the (de)compression(default is RLE).\n");
    printf("  -d\t Perform a decompression rather than a compression to the output file..\n");
}


void RLE_Decode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;
    BYTE c,length;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    length = readByte(in);
    c = readByte(in);

    while (!feof(in)){
        for(; length > 0; --length)
            writeByte(c,out);

        length = readByte(in);
        c = readByte(in);
    }

    fclose(in);
    fclose(out);
}

void RLE_Encode(char * inputfile,char * outputfile)
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

void writeRawPacket(BYTE length,BYTE * data,FILE * fp)
{
    BYTE packetHead;
    int i;

    packetHead = 0;
    packetHead |= length ;
    writeByte(packetHead,fp);
    for(i = 0 ; i < (length + 1) ; ++i)
        writeByte(data[i],fp);
}

void writeRunLengthPacket(BYTE length,BYTE data,FILE * fp)
{
    BYTE packetHead;

    packetHead = 0x80;
    packetHead |= length;
    writeByte(packetHead,fp);
    writeByte(data,fp);
}


void packBitsEncode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;

    BYTE c2,c1;
    BYTE length;
    int passedFirstCharacter;
    int packetType;
    BYTE data[128];

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    length = 0;
    passedFirstCharacter = 0;

    packetType = RAW_PACKET;
    c2 = readByte(in);

    while (!feof(in)){
        /* if it's not the first character. */
        if(passedFirstCharacter){

            /* if the packet is full.*/
            if(length == 127){
                if(packetType == RUN_LENGTH_PACKET){
                    writeRunLengthPacket(length,c1,out);
                    packetType = RAW_PACKET;
                    length = 0;
                }else{
                    writeRawPacket(length-1,data,out);
                    length = 1;
                }
            }
            else if(c2 == c1){
                /* if a raw packet was being written, finish it. */
                if(packetType == RAW_PACKET && length > 0){
                    writeRawPacket(length-1,data,out);
                    length = 0;
                }
                ++length;
                packetType = RUN_LENGTH_PACKET;
            }
            /* if it's a raw packet, or the run length packet is full. */
            else{
                if(packetType == RUN_LENGTH_PACKET){
                    writeRunLengthPacket(length,c1,out);
                    packetType = RAW_PACKET;
                    length = 0;
                }else{
                    /* continue raw packet */
                    data[length] = c1;
                    length = length + 1;
                    packetType = RAW_PACKET;
                }
            }
        }
        passedFirstCharacter = 1;
        c1 = c2;
        c2 = readByte(in);
    }

/* write the last bytes. */
    if(passedFirstCharacter){
        if(packetType == RUN_LENGTH_PACKET){
            writeRunLengthPacket(length,c1,out);
        } else{
            data[length] = c1;
            writeRawPacket(length,data,out);
        }
    }

    fclose(in);
    fclose(out);
}

void packBitsDecode(char * inputfile,char * outputfile)
{
    FILE * in;
    FILE * out;
    BYTE b,length,head;

    in = fopen(inputfile,"rb");
    out = fopen(outputfile,"wb");

    assertFileOpened(in);
    assertFileOpened(out);


    head = readByte(in);

    while (!feof(in)){


        length = head & 0x7f;

        /* run length packet */
        if(head & 0x80){
            b = readByte(in);

            for(length = length + 1; length > 0; --length)
                writeByte(b,out);
        } else {
            /* raw packet */
            for(length = length + 1; length > 0; --length){
                b = readByte(in);
                writeByte(b,out);
            }
        }

        head = readByte(in);
    }

    fclose(in);
    fclose(out);
}


