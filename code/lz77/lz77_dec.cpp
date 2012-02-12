#include "lz77_dec.h"
#include "token.h"
#include "../bits.h"

#include <vector>
#include <cmath>

using std::vector;

void decodeToken(Token token, vector<BYTE> & decompressed);

void decodeToken(Token token, vector<BYTE> & decompressed)
{
    for(unsigned int i = 0; i < token.length; ++i){
        decompressed.push_back(decompressed[decompressed.size() - token.offset]);
    }

    decompressed.push_back(token.symbol);
}

void decompress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    FILE * in,
    FILE * out)
{
    unsigned int offsetSize = ceil(log2(windowSize));
    unsigned int lengthSize = ceil(log2(lookAheadSize-1));

    BitFileReader inBits(in,MSBF);

    /* For the sake of simplicity.*/
    vector<BYTE> decompressed;

    size_t size = inBits.readBits(64);

    while(decompressed.size() < size){

        Token token;

        token.offset = inBits.readBits(offsetSize);
        token.length = inBits.readBits(lengthSize);
        token.symbol = inBits.readBits(symbolSize);


        decodeToken(token, decompressed);
    }

    for(size_t i = 0; i < decompressed.size(); ++i){
        putc(decompressed[i], out);
    }
}
