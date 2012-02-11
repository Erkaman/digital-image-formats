#include <cstdio>
#include <vector>
#include <cmath>
#include <cstring>
#include <deque>
#include "../defs.h"
#include "../bits.h"
#include "../io.h"
#include <iostream>
#include <iterator>

using std::deque;
using std::ostream_iterator;
using std::cout;
using std::vector;

const int symbolSize = 8;

void compress(FILE * in, FILE * out);

void decompress(FILE * in, FILE * out);

void maintainSize(
    unsigned int windowSize,
    deque<BYTE> & window,
    deque<BYTE> & lookahead,
    unsigned int matchLength );

class Token{

public:

    unsigned int offset;
    unsigned int length;
    BYTE symbol;
};

void printBuffer(const deque<BYTE> & buffer);

void decodeToken(Token token, vector<BYTE> & decompressed);

Token searchWindow(
    deque<BYTE> & window,
    deque<BYTE> & lookahead,
    unsigned int windowSize);

class TokenWriter{

private:

    int offsetSize;
    int lengthSize;

    BitFileWriter outBits;

public:

    TokenWriter(int windowSize, int lookAheadSize, FILE * out):
        offsetSize(ceil(log2(windowSize))),
        lengthSize(log2(lookAheadSize-1)),
        outBits(out,MSBF) {
    }

    void writeToken(const Token & token){
	printf("(%d,%d,%c)\n", token.offset, token.length, token.symbol);
        outBits.writeBits(token.offset, offsetSize);
        outBits.writeBits(token.length, lengthSize);
        outBits.writeBits(token.symbol, symbolSize);
    }

    void flush(){
	outBits.writeBits(0, 8);
    }

    BitFileWriter getOutBits()const{
	return outBits;
    }

};

class TokenReader{

private:

    int offsetSize;
    int lengthSize;

    BitFileReader inBits;

public:

    TokenReader(int windowSize, int lookAheadSize, FILE * in):
        offsetSize(ceil(log2(windowSize))),
        lengthSize(log2(lookAheadSize-1)),
        inBits(in,MSBF) {
    }

    BitFileReader getInBits()const {
	return inBits;
    }

    Token readToken(){

	Token token;

	token.offset = inBits.readBits(offsetSize);
	token.length = inBits.readBits(lengthSize);
	token.symbol = inBits.readBits(symbolSize);

	printf("(%d,%d,%c)\n", token.offset, token.length, token.symbol);

	return token;
    }

};



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
    const unsigned int windowSize = pow(2,12);
    const unsigned int lookAheadSize = pow(2,4);

    TokenReader tokenIn(windowSize, lookAheadSize, in);

    /* For the sake of simplicity.*/
    vector<BYTE> decompressed;

    size_t size = tokenIn.getInBits().readBits(64);

    printf("size:%ld\n", size);

    while(decompressed.size() < size){
	Token token = tokenIn.readToken();

	decodeToken(token, decompressed);
     }

    for(size_t i = 0; i < decompressed.size(); ++i){
	putc(decompressed[i], out);
    }

 }

 void maintainSize(
     unsigned int windowSize,
     deque<BYTE> & window,
     deque<BYTE> & lookahead,
     unsigned int matchLength){

     for(size_t i = 0; i < matchLength; ++i){

	 /* Move the first element of the lookahead
	    buffer to the window buffer */
	 window.push_back(lookahead.front());

	 lookahead.pop_front();

	 /* Ensure the size of the window buffer. */
	 if(window.size() > windowSize)
	     window.pop_front();
     }

 }

 void compress(FILE * in, FILE * out)
 {
     const unsigned int windowSize = pow(2,12);
     const unsigned int lookAheadSize = pow(2,4);
     bool endInput = false;

     TokenWriter tokenOut(windowSize, lookAheadSize, out);

     deque<BYTE> window;
     deque<BYTE> lookahead;

     size_t fs = getFileSize(in);

     tokenOut.getOutBits().writeBits(fs, 64);

     do{

	 if(!endInput){
	     int ch = getc(in);
	     if(ch == EOF)
		 endInput = true;
	     else{
/*		 printf("ch:%c\n", ch); */
		 lookahead.push_back(ch);
		 if(lookAheadSize < lookahead.size()){
		     maintainSize(windowSize, window, lookahead,1);
		 }
	     }
	 }

	 if(
	     lookahead.size() == lookAheadSize ||

	     /* Process the last few characters in the file. */
	     (lookahead.size() < lookAheadSize && endInput)){


 /*	    printf("window\n");
	     printBuffer(window);

	     printf("lookahead\n");
	     printBuffer(lookahead); */

	     Token token = searchWindow(window, lookahead, windowSize);
	     tokenOut.writeToken(token);

	 }
     } while(lookahead.size() != 0);

     tokenOut.flush();
 }

 Token searchWindow(
     deque<BYTE> & window,
     deque<BYTE> & lookahead,
     unsigned int windowSize)
 {
     Token token;

     token.offset = 0;
     token.length = 0;

     for(
	 deque<BYTE>::const_reverse_iterator windowIter = window.rbegin();
	 windowIter != window.rend();
	 ++windowIter){

	 /* Match first character*/
	 if(lookahead.front() == *windowIter){

	     /* Brute force match as many character as possible.*/

	     Token candidate;
	     candidate.offset = (windowIter + 1) - window.rbegin();
	     candidate.length = 1;

	     for(
		 deque<BYTE>::const_reverse_iterator searchIter = (windowIter - 1);

		 searchIter != window.rbegin() &&

		     /* Stop matching if the end of the lookahead
			buffer has been reached.*/
		     (lookahead.begin() + candidate.length) != lookahead.end();
		 --searchIter){

		 printf("cmp1:%c\n", *searchIter);
		 printf("cmp2:%c\n", *(lookahead.begin() + candidate.length));


		 if(*searchIter !=
		    *(lookahead.begin() + candidate.length))
		     break;

		 ++candidate.length;

	     }

	     /* Don't encode single length strings because it's just silly and makes the debugging output harder to read.*/
	     if(candidate.length == 1){
		 token.offset = 0;
		 token.length = 0;
	     } else if(candidate.length > token.length)
		 token = candidate;
	 }
     }

     maintainSize(windowSize, window, lookahead, token.length);

     token.symbol = lookahead.front();

     maintainSize(windowSize, window, lookahead, 1);

     return token;
 }


 void printBuffer(const deque<BYTE> & buffer){
     copy(
	 buffer.begin(),
	 buffer.end(),
	 ostream_iterator<BYTE>(cout, ", "));

     cout << "\n";
}

void decodeToken(Token token, vector<BYTE> & decompressed)
{
     for(unsigned int i = 0; i < token.length; ++i){
	 decompressed.push_back(decompressed[decompressed.size() - token.offset]);
    }

     decompressed.push_back(token.symbol);
}
