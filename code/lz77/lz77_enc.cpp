#include "lz77_enc.h"
#include "token.h"
#include "../bits.h"
#include "../io.h"
#include "../stlutil.h"

#include <deque>
#include <iterator>
#include <iostream>
#include <cmath>

using std::ostream_iterator;
using std::deque;
using std::cout;
using std::vector;

void printBuffer(const deque<BYTE> & buffer);

unsigned int windowCmp(
    deque<BYTE> & buffer,
    unsigned int i,
    unsigned int j,
    unsigned int length);

Token searchWindow(
    unsigned int windowSize,
    unsigned int & lookAheadBeg,
    std::deque<BYTE> & buffer);

void printBuffer(const deque<BYTE> & buffer){

    for(size_t i = 0; i < buffer.size(); ++i){
        printf("%ld:%c,", i, buffer[i]);
    }

    printf("\n");
}

unsigned int windowCmp(
    deque<BYTE> & buffer,
    unsigned int i,
    unsigned int j,
    unsigned int length)
{
    unsigned int count = 0;

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
    unsigned int windowSize,
    unsigned int & lookAheadBeg,
    deque<BYTE> & buffer)
{
    Token token;

    token.offset = 0;
    token.length = 0;

    /* Special case the last character. */
    if( ((unsigned int)buffer.size() - lookAheadBeg) == 1 ){
        token.symbol = buffer[lookAheadBeg];
	++lookAheadBeg;
        return token;
    }
    /* Because the max value of a n-bit number is (2^n)-1 */
    for(unsigned int windowI = 1; windowI < lookAheadBeg; ++windowI){

        unsigned int length = windowCmp(
            buffer,
            windowI,
            lookAheadBeg, ((unsigned int)buffer.size() - lookAheadBeg) - 2);

        if(length > token.length){
            token.offset = lookAheadBeg - windowI;
            token.length = length;
        }
    }

    for(unsigned int i = 0; i < token.length; ++i)
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

vector<Token> compress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    const vector<BYTE> & data);

vector<Token> compress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    const vector<BYTE> & data)
{
    bool endData = false;

    deque<BYTE> buffer;

    unsigned int lookAheadBeg = 0;

    vector<Token> compressed;

    vector<BYTE>::const_iterator iter = data.begin();
    const vector<BYTE>::const_iterator End = data.end();

    do{
        if(!endData){
            if(iter == End)
                endData = true;
            else{
		BYTE b = *iter;
		++iter;

                buffer.push_back(b);

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
            endData) {

            Token token =
                searchWindow(windowSize, lookAheadBeg, buffer);

	    compressed.push_back(token);
        }

    } while(
        /* While the entire lookahead buffer hasn't yet been processed. */
        (buffer.size() - lookAheadBeg) > 0);

    return compressed;
}

void compress(unsigned int windowSize, unsigned int lookAheadSize, FILE * in, FILE * out)
{
    vector<BYTE> file = readFileBytes(in);

    vector<Token> compressed =
	compress(windowSize,lookAheadSize,file);

    BitFileWriter outBits(out, MSBF);

    int offsetSize = ceil(log2(windowSize));
    int lengthSize = ceil(log2(lookAheadSize-1));

    outBits.writeBits(file.size(), 64);

    for(size_t i = 0; i < compressed.size(); ++i){
	Token token = compressed[i];
        outBits.writeBits(token.offset, offsetSize);
        outBits.writeBits(token.length, lengthSize);
        outBits.writeBits(token.symbol, 8);
    }

    /* Flush*/
    outBits.writeBits(0, 8);
}
