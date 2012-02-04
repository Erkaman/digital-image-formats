#include <cstdio>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <cstring>
#include <deque>
#include "../defs.h"
#include "../bits.h"

using std::deque;

void compress(FILE * in, FILE * out);

void decompress(FILE * in, FILE * out);



class Token{

public:

    int offset;
    int length;
    BYTE symbol;
};

Token searchWindow(
    deque<BYTE> buffer,
    unsigned int lookAheadSize);

class TokenWriter{

private:

    int offsetSize;
    int lengthSize;
    static const int symbolSize;

    BitWriter outBits;

public:

    TokenWriter(int windowSize, int lookAheadSize, FILE * out):
        offsetSize(ceil(log2(windowSize))),
        lengthSize(log2(lookAheadSize-1)),
        outBits(out,MSBF) {
    }

    void writeToken(const Token & token){
        outBits.writeBits(token.offset, offsetSize);
        outBits.writeBits(token.length, lengthSize);
        outBits.writeBits(token.symbol, symbolSize);
    }

};

const int TokenWriter::symbolSize = 8;

int main(int argc, char *argv[])
{
    if(argc == 1){
        printf("No files specified\n");
        return 1;
    } else{

        FILE * in, * out;

        if(!strcmp(argv[1], "-d") && (argc == 4)){
            in = fopen(argv[2], "rb");
            out = fopen(argv[3], "wb");

            decompress(in,out);
        } else {
            in = fopen(argv[1], "rb");
            out = fopen(argv[2], "wb");
            compress(in,out);
        }

        fclose(in);
        fclose(out);
    }

}

void decompress(FILE * in, FILE * out)
{
    in = in;
    out = out;
}


void compress(FILE * in, FILE * out)
{
    const unsigned int windowSize = pow(2,12);
    const unsigned int lookAheadSize = pow(2,4);
    bool endInput = false;

    TokenWriter tokenOut(windowSize, lookAheadSize, out);

    deque<BYTE> buffer;

    do{

        if(!endInput){
            int ch = getc(in);
            if(ch == EOF)
                endInput = true;
	    else{
		printf("ch:%d\n", ch);
		buffer.push_front(ch);
	    }
        }

	if(buffer.size() >= lookAheadSize){


/*	    Token token = searchWindow(buffer, lookAheadSize);
	    tokenOut.writeToken(token);

	    if(token.length == 0 && token.offset == 0){

	    } */
	} else if(endInput) {

	    printf("aaa\n");

	    Token token = searchWindow(buffer, lookAheadSize);
	    tokenOut.writeToken(token);

	    if(token.length == 0 && token.offset == 0){

	    }

	}

    } while(buffer.size() != 0);
}

Token searchWindow(
    deque<BYTE> buffer,
    unsigned int lookAheadSize)
{
    Token token;
    signed long firstMatchI = buffer.size() - lookAheadSize;

    if(firstMatchI < 0){
	firstMatchI = 0;
    }

    token.offset = 0;
    token.length = 0;

    signed long searchBufferSize = firstMatchI;

    for(signed long i = 0; i < searchBufferSize; ++i){

    }

    token.symbol = buffer[firstMatchI];

    return token;
}
