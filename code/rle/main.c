#include "main.h"

int main(int argc, char *argv[])
{
    int decompress = 0;
    int optimize = 0;
    FILE * in;
    FILE * out;

    verbose = 0;

    if(argc == 1){
        printf("No arguments specified.\n");
        printf("Try --help for more information.\n");
        return 1;
    }
g
    /* parse the command line arguments */

    ++argv;
    --argc;
    while(1){

        if(!strcmp("--help",*argv)){
            printHelp();
            return 0;
        }
        else if(!strcmp("-d",*argv))
            decompress = 1;
        else if(!strcmp("-p",*argv))
            optimize = 1;
        else if(!strcmp("-v",*argv))
            verbose = 1;
        else
            break;

        ++argv;
	--argc;
    }

    if(argc < 1){
        printf("An input AND an output file must be specified.");
        return 1;
    }


    in = fopen(argv[0],"rb");
    out = fopen(argv[1],"wb");

    assertFileOpened(in);
    assertFileOpened(out);

    if(optimize){
        if(decompress)
            packBitsDecode(in,out);
        else
            packBitsEncode(in,out);
    } else{
        if(decompress)
            RLE_Decode(in,out);
        else
            RLE_Encode(in,out);
    }

    fclose(in);
    fclose(out);

    return 0;
}

void printHelp(void)
{
    printf("Usage: rle [OPTION]... IN OUT\n");
    printf("Compress or decompress the IN file to the OUT file using the Run Length Encoding/Decoding algorithm.\n");

    printf("  --help\tDisplay this help message.\n");
    printf("  -p\tUse the packBits algorithm for the (de)compression(default is plain RLE).\n");
    printf("  -d\t Perform decompression rather than compression to the output file.\n");
}

void RLE_Decode(FILE * in, FILE * out)
{
    BYTE c,length;

    verbosePrint("RLE Decoding\n");

    length = readByte(in);
    c = readByte(in);

    while (!feof(in)){

	verbosePrint("RLE Packet: data = %d, size = %d\n", c, length);
        for(; length > 0; --length)
            writeByte(c,out);

        length = readByte(in);
        c = readByte(in);
    }
}

void RLE_Encode(FILE * in, FILE * out)
{
    BYTE c2,c1;
    BYTE length;
    int passedFirstCharacter;

    verbosePrint("RLE Encoding\n");

    length = 1;
    passedFirstCharacter = 0;

    c2 = readByte(in);
    while (!feof(in)){

        /* if it's not the first character. */
        if(passedFirstCharacter){
            if(c2 == c1 && length < BYTE_MAX)
                ++length;
            else{

		verbosePrint("RLE Packet: data = %d, size = %d\n", c1, length);

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
	verbosePrint("RLE Packet: data = %d, size = %d\n", c1, length);
        writeByte(length,out);
        writeByte(c1,out);
    }
}

void writeRawPacket(BYTE length,BYTE * data,FILE * out)
{
    BYTE packetHead;
    int i;

    packetHead = 0;
    packetHead |= length;
    writeByte(packetHead,out);

    verbosePrint("Packbits Raw Packet: size = %d,contents:\n", length + 1);

    for(i = 0 ; i < (length + 1) ; ++i){
        writeByte(data[i],out);
	verbosePrint("%d\n",data[i]);
    }
}

void writeRunLengthPacket(BYTE length,BYTE data,FILE * out)
{
    BYTE packetHead;

    packetHead = 0x80;
    packetHead |= length;
    writeByte(packetHead,out);
    writeByte(data,out);

    verbosePrint("Packbits RLE Packet: data = %d, size = %d\n", data, length + 1);
}

void packBitsEncode(FILE * in, FILE * out)
{
    BYTE c2,c1;
    BYTE length;
    int passedFirstCharacter;
    int packetType;
    BYTE data[128];

    verbosePrint("RLE Packbits Encoding\n");

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
}

void packBitsDecode(FILE * in, FILE * out)
{
    BYTE b,length,head;

    verbosePrint("RLE Packbits Decoding\n");

    head = readByte(in);

    while (!feof(in)){


        length = head & 0x7f;

        /* run length packet */
        if(head & 0x80){
            b = readByte(in);

	    verbosePrint("Packbits RLE Packet: data = %d, size = %d\n", b, length + 1);

            for(length = length + 1; length > 0; --length)
                writeByte(b,out);
        } else {
            /* raw packet */

	    verbosePrint("Packbits Raw Packet: size = %d, contents:\n", length + 1);

            for(length = length + 1; length > 0; --length){

                b = readByte(in);
                writeByte(b,out);

		verbosePrint("%d\n",b);
            }
        }

        head = readByte(in);
    }
}
