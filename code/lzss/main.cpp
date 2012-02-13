#include "../bits.h"
#include "lzss_util.h"
#include "../stlutil.h"

#include <cstdio>
#include <cstring>
#include <cmath>

using std::vector;

#define SYMBOL 1
#define OFFSET_LENGTH 0

void compress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    FILE * in,
    FILE * out);

void decompress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    FILE * in,
    FILE * out);


int main(int argc, char *argv[])
{
    if(argc == 1){
        printf("No files specified\n");
        return 1;
    } else{

        FILE * in, * out;

        const signed long windowSize = pow(2,11);
        const signed long lookAheadSize = pow(2,4);

        if(!strcmp(argv[1], "-d") && (argc == 4)){
            in = fopen(argv[2], "rb");
            out = fopen(argv[3], "wb");

            decompress(windowSize, lookAheadSize, in,out);
        } else {
            in = fopen(argv[1], "rb");
            out = fopen(argv[2], "wb");
            compress(windowSize, lookAheadSize, in,out);
        }

        fclose(in);
        fclose(out);
    }
}

void compress(unsigned int windowSize, unsigned int lookAheadSize, FILE * in, FILE * out)
{
    vector<BYTE> file = readFileBytes(in);

    vector<Token> compressed =
        compress(windowSize,lookAheadSize,file);

    printf("compressor:\n");

    BitFileWriter outBits(out, MSBF);

    int offsetSize = ceil(log2(windowSize));
    int lengthSize = ceil(log2(lookAheadSize-1));

    outBits.writeBits(file.size(), 32);

    for(size_t i = 0; i < compressed.size(); ++i){
        Token token = compressed[i];

        if(token.type == SymbolToken){

            outBits.writeBits(SYMBOL, 1);
            outBits.writeBits(token.symbol, 8);

        } else if(token.type == OffsetLengthToken) {

            outBits.writeBits(OFFSET_LENGTH, 1);

            outBits.writeBits(token.offset, offsetSize);
            outBits.writeBits(token.length, lengthSize);
        }
    }

    /* Flush*/
    outBits.writeBits(0, 8);
}

void decompress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    FILE * in,
    FILE * out)
{
    printf("decompressor:\n");

    unsigned int offsetSize = ceil(log2(windowSize));
    unsigned int lengthSize = ceil(log2(lookAheadSize-1));

    BitFileReader inBits(in,MSBF);

    /* For the sake of simplicity.*/
    vector<BYTE> decompressed;

    size_t size = inBits.readBits(32);

    while(decompressed.size() < size){

        Token token;

        int flag = inBits.readBits(1);

        if(flag == SYMBOL){

            token.symbol = inBits.readBits(8);

            decompressed.push_back(token.symbol);

        } else if(flag == OFFSET_LENGTH){

            token.offset = inBits.readBits(offsetSize);
            token.length = inBits.readBits(lengthSize);

            decodeToken(token, decompressed);
        }
    }

    for(size_t i = 0; i < decompressed.size(); ++i){
        putc(decompressed[i], out);
    }
}
