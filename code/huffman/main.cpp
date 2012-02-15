#include "../frequency_table.h"
#include "../io.h"
#include "../bits.h"
#include "../stlutil.h"

#include "huffman.h"

#include <vector>
#include <cstring>
#include <map>
#include <iterator>
#include <algorithm>

using std::vector;
using std::map;
using std::upper_bound;

void printHelp(void);

void huffmanCompress(FILE * in,FILE * out);
void huffmanDecompress(FILE * in,FILE * out);



int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;
    int decompress = 0;

    verbose = 1;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
        return 0;
    }

    /* do the command line parsing */

    ++argv;
    --argc;
    while(1){

        if(!strcmp("--help",*argv)){
            printHelp();
            return 0;
        }
        else if(!strcmp("-d",*argv))
            decompress = 1;
        else if(!strcmp("-v",*argv))
            verbose = 1;
        else
            break;

        ++argv;
        --argc;
    }

    if(argc < 2){
        printf("An input AND an output file must be specified.");
        return 1;
    }

    in = fopen(argv[0],"rb");
    assertFileOpened(in);

    out = fopen(argv[1],"wb");
    assertFileOpened(out);

    if(decompress)
        huffmanDecompress(in,out);
    else
        huffmanCompress(in,out);

    fclose(in);
    fclose(out);

    return 0;
}

void printHelp(void)
{
    printf("Usage: huffman IN\n");
    printf("Compress or decompress a Huffman coded file.\n"
           "Huffman Coding is done on a file by default");
    printf("  --help\tDisplay this help message.\n");
    printf("  -d\tPerform decompression.\n");
    printf("  -v\tVerbose output.\n");
}

void huffmanCompress(FILE * in,FILE * out)
{
    vector<unsigned int> fileData = readFile(in);

    CodeLengths byteCodeLengths = makeCodeLengths(fileData, 256, 15);

    CodesList byteCodes = translateCodes(byteCodeLengths);

    CodeLengths codeLengthalphabetCodeLengths;
    CodeLengths compressedByteCodeLengths = compressCodeLengths(
        byteCodeLengths,
        codeLengthalphabetCodeLengths);

    CodeLengths codeLengthCodeLengths =
        makeCodeLengths(codeLengthalphabetCodeLengths, 19,7);
    CodesList codeLengthCodes = translateCodes(codeLengthCodeLengths);

    BitFileWriter * outBits = new BitFileWriter(out, LSBF);

    writeCodeLengthCodeLengths(codeLengthCodeLengths, outBits);

    writeCompressedCodeLengths(
	compressedByteCodeLengths,
	codeLengthCodes,
	outBits);

    outBits->writeBits(fileData.size(), 64);

    for(
        vector<unsigned int>::const_iterator iter = fileData.begin();
        iter != fileData.end();
        ++iter){
        int c = *iter;

        HuffmanCode code = byteCodes[c];
        writeCode(code, outBits);
    }

    outBits->writeBits(0,8);

    delete outBits;
}

void huffmanDecompress(FILE * in,FILE * out)
{
    BitFileReader * inBits = new BitFileReader(in, LSBF);

    unsigned int HCLEN =  inBits->readBits(4);

    RevCodesList codeLengthCodeLengths = loadCodeLengthCodes(HCLEN, inBits);

    /* The code lengths are compressed! */

    RevCodesList lookup = loadUsingCodeLengthCodes(
        256,
        256,
        codeLengthCodeLengths,
        inBits);

    size_t fileSize = inBits->readBits(64);

    for(size_t i = 0; i < fileSize; ++i){
        putc(readCode(lookup, inBits),out);
    }

    delete inBits;
}

