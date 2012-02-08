#include <cstring>
#include <cstdlib>
#include <vector>
#include "zlib.h"
#include "../io.h"

using std::vector;

void printHelp(void);

void ZLIB_DecompressFile(FILE * in, FILE * out);

vector<BYTE> getCompressedData(FILE * in);

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;
    int decompress = 0;

    verbose = 0;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
        return 0;
    }

    ++argv;
    while(1){

        if(!strcmp("--help",*argv)){
            printHelp();
            return 0;
        }
        else if(!strcmp("-d",*argv))
            decompress = 1;
        else if(!strcmp("-v",*argv)){
	    printf("lol\n");
            verbose = 1;
	    }
        else
            break;

        ++argv;
    }

    in = fopen(argv[0],"rb");
    assertFileOpened(in);

    out = fopen(argv[1],"wb");
    assertFileOpened(out);

    if(decompress){
        ZLIB_DecompressFile(in,out);
    }else{
	printf("DEFLATE Compression not yet implemented.\n");
    }

    fclose(in);
    fclose(out);

    return 0;
}

void printHelp(void)
{
    printf("Usage: deflate IN OUT\n");
    printf("Compress or decompress a DEFLATE compressed file.\n");
    printf("  --help\tDisplay this help message.\n");
    printf("  -d\tPerform decompression.\n");
    printf("  -v\tVerbose output.\n");
}

void ZLIB_DecompressFile(FILE * in, FILE * out)
{
    vector<BYTE> decompressed, compressed;

    compressed = getCompressedData(in);

    verbosePrint("Read in compressed data, size: %ld\n",compressed.size());
/*    printData(compressed); */

    decompressed = ZLIB_Decompress(compressed);

    decompressed = decompressed;
    out = out;

    for(size_t i = 0; i < decompressed.size(); ++i)
	putc(decompressed[i], out);
}

vector<BYTE> getCompressedData(FILE * in)
{
    vector<BYTE> data;
    int ch;

    while(true){
	ch = getc(in);
	if(ch == EOF)
	    break;

	data.push_back(ch);
    }

    return data;
}

