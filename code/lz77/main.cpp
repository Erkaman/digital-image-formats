#include <cstdio>
#include <cstring>
#include "lz77_dec.h"
#include "lz77_enc.h"
#include <cmath>

int main(int argc, char *argv[])
{
    if(argc == 1){
        printf("No files specified\n");
        return 1;
    } else{

        FILE * in, * out;

	const signed long windowSize = pow(2,8);
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
