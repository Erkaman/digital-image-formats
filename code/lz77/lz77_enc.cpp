#include "lz77_enc.h"
#include "token.h"
#include "../bits.h"
#include "../io.h"

#include <deque>
#include <iterator>
#include <iostream>
#include <cmath>

using std::ostream_iterator;
using std::deque;
using std::cout;

void printBuffer(const deque<BYTE> & buffer);

signed long windowCmp(
    deque<BYTE> & buffer,
    signed long i,
    signed long j,
    signed long length);

Token searchWindow(
    int windowSize,
    signed long & lookAheadBeg,
    std::deque<BYTE> & buffer);

class TokenWriter{

private:

    int offsetSize;
    int lengthSize;

    BitFileWriter outBits;

public:

    TokenWriter(int windowSize, int lookAheadSize, FILE * out);

    void writeToken(const Token & token){
        printf("(%ld,%ld,%c)\n", token.offset, token.length, token.symbol);
        outBits.writeBits(token.offset, offsetSize);
        outBits.writeBits(token.length, lengthSize);
        outBits.writeBits(token.symbol, symbolSize);
    }

    void flush();

    BitFileWriter getOutBits()const;

};

TokenWriter::TokenWriter(int windowSize, int lookAheadSize, FILE * out):
    offsetSize(ceil(log2(windowSize))),
    lengthSize(ceil(log2(lookAheadSize-1))),
    outBits(out,MSBF) {
}

void TokenWriter::flush(){
    outBits.writeBits(0, 8);
}

BitFileWriter TokenWriter::getOutBits()const{
    return outBits;
}

void printBuffer(const deque<BYTE> & buffer){

    for(size_t i = 0; i < buffer.size(); ++i){
        printf("%ld:%c,", i, buffer[i]);
    }

    printf("\n");
}

signed long windowCmp(
    deque<BYTE> & buffer,
    signed long i,
    signed long j,
    signed long length)
{
    signed long count = 0;

    while(length > 0){
        --length;
        if(buffer[i++] == buffer[j++])
            ++count;
        else
            return count;
    }

    return count;
}

Token searchWindow(
    int windowSize,
    signed long & lookAheadBeg,
    deque<BYTE> & buffer)
{
    Token token;

    token.offset = 0;
    token.length = 0;

    /* Special case the last character. */
    if( ((signed long)buffer.size() - lookAheadBeg) == 1 ){
        token.symbol = buffer[lookAheadBeg];
	++lookAheadBeg;
        return token;
    }
    /* Because the max value of a n-bit number is (2^n)-1 */
    for(signed long windowI = 1; windowI < lookAheadBeg; ++windowI){

        signed long length = windowCmp(
            buffer,
            windowI,
            lookAheadBeg, ((signed long)buffer.size() - lookAheadBeg) - 2);

        if(length > token.length){
            token.offset = lookAheadBeg - windowI;
            token.length = length;
        }
    }

    for(signed long i = 0; i < token.length; ++i)
        ++lookAheadBeg;

    token.symbol = buffer[lookAheadBeg];

    ++lookAheadBeg;

    while(
        lookAheadBeg > windowSize){
        buffer.pop_front();
        --lookAheadBeg;
    }

    return token;
}

void compress(signed long windowSize, signed long lookAheadSize, FILE * in, FILE * out)
{
    bool endInput = false;

    TokenWriter tokenOut(windowSize, lookAheadSize, out);

    deque<BYTE> buffer;

    signed long fs = getFileSize(in);

    signed long lookAheadBeg = 0;

    tokenOut.getOutBits().writeBits(fs, 64);

    do{
        if(!endInput){
            int ch = getc(in);
            if(ch == EOF)
                endInput = true;
            else{
/*                printf("ch:%c\n", ch);*/
                buffer.push_back(ch);

		while(lookAheadBeg > lookAheadSize){
		    buffer.pop_front();
		    --lookAheadBeg;
		}
            }
        }

        if(
            /* If the lookahead buffer is currently full then start matching.*/
            lookAheadSize == (buffer.size() - lookAheadBeg) ||

            /* Or if the end of the input been reached then match the
             * rest of the lookahead buffer */
            endInput) {

            Token token =
                searchWindow(windowSize, lookAheadBeg, buffer);
            tokenOut.writeToken(token);

        }

/*	printf("lookaheadsize:%ld\n", buffer.size() - lookAheadBeg); */

    } while(
        /* While the entire lookahead buffer hasn't yet been processed. */
        ((signed long)buffer.size() - lookAheadBeg) > 0);

    tokenOut.flush();
}

