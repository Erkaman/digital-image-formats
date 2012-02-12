#include "lz77_dec.h"
#include "token.h"
#include "../bits.h"

#include <vector>
#include <cmath>

using std::vector;

void decodeToken(Token token, vector<BYTE> & decompressed);

class TokenReader{

private:

    int offsetSize;
    int lengthSize;

    BitFileReader inBits;

public:

    TokenReader(int windowSize, int lookAheadSize, FILE * in);

    BitFileReader getInBits()const;

    Token readToken();
};

TokenReader::TokenReader(int windowSize, int lookAheadSize, FILE * in):
    offsetSize(ceil(log2(windowSize))),
    lengthSize(ceil(log2(lookAheadSize-1))),
    inBits(in,MSBF)
{
}

BitFileReader TokenReader::getInBits()const
{
    return inBits;
}

Token TokenReader::readToken(){

    Token token;

    token.offset = inBits.readBits(offsetSize);
    token.length = inBits.readBits(lengthSize);
    token.symbol = inBits.readBits(symbolSize);

/*    printf("(%ld,%ld,%c)\n", token.offset, token.length, token.symbol); */

    return token;
}

void decodeToken(Token token, vector<BYTE> & decompressed)
{
    for(signed long i = 0; i < token.length; ++i){
        decompressed.push_back(decompressed[decompressed.size() - token.offset]);
    }

    decompressed.push_back(token.symbol);
}

void decompress(signed long windowSize, signed long lookAheadSize, FILE * in, FILE * out)
{
    TokenReader tokenIn(windowSize, lookAheadSize, in);

    /* For the sake of simplicity.*/
    vector<BYTE> decompressed;

    size_t size = tokenIn.getInBits().readBits(64);

/*    printf("size:%ld\n", size); */

    while(decompressed.size() < size){
        Token token = tokenIn.readToken();

        decodeToken(token, decompressed);
    }

    for(size_t i = 0; i < decompressed.size(); ++i){
        putc(decompressed[i], out);
    }

}


