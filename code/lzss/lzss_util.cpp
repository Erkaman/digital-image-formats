#include "lzss_util.h"


#include <deque>
#include <iterator>
#include <iostream>
#include <cmath>
#include <vector>
#include <cstdio>

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

                while(lookAheadBeg > windowSize){
                    buffer.pop_front();
                    --lookAheadBeg;
                }
            }
        }
        if(endData){
            while(lookAheadBeg > windowSize){
                buffer.pop_front();
                --lookAheadBeg;
            }
        }

        /* If the lookahead buffer is currently full then start matching.*/
        /* Or if the end of the input been reached then match the
         * rest of the lookahead buffer */
        if(lookAheadSize == (buffer.size() - lookAheadBeg) || endData) {

            Token token;

            token.offset = 0;
            token.length = 0;

            unsigned int windowI;

            /* If the window is filled then is not enough storage
             * space for an offset to the very first character in the
             * window. */
            if(lookAheadBeg == windowSize)
                windowI = 1;
            else
                windowI = 0;

            for(windowI = windowI; windowI < lookAheadBeg; ++windowI){

                unsigned int length = windowCmp(
                    buffer,
                    windowI,
                    lookAheadBeg, ((unsigned int)buffer.size() - lookAheadBeg) - 1);

                if(length > token.length){
                    token.offset = lookAheadBeg - windowI;
                    token.length = length;
                }
            }


            if(token.length > 1) {
                token.type = OffsetLengthToken;
            } else{
                token.symbol = buffer[lookAheadBeg];
                token.type = SymbolToken;
                token.length = 1;
            }

            for(unsigned int i = 0; i < token.length; ++i)
                ++lookAheadBeg;

/*            if(token.type == SymbolToken){
              printf("(%c)\n",token.symbol);
              } else if(token.type == OffsetLengthToken){
              printf("(%d,%d)\n",token.offset, token.length);
              } */

            compressed.push_back(token);
        }

    } while(
        /* While the entire lookahead buffer hasn't yet been processed. */
        (buffer.size() - lookAheadBeg) > 0);

    return compressed;
}

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

    while(length--){
        if( buffer[i++] == buffer[j++])
            ++count;
        else
            return count;
    }

    return count;
}

void decodeToken(Token token, vector<BYTE> & decompressed)
{
    for(unsigned int i = 0; i < token.length; ++i){
        decompressed.push_back(decompressed[decompressed.size() - token.offset]);
    }
}
