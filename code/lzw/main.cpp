#include "../io.h"
#include "../bits.h"
#include "../stlutil.h"
#include "lzwutil.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <map>
#include <vector>
#include <cmath>

using std::pair;
using std::vector;
using std::map;
using std::pow;

void printHelp(void);

void LZW_Compress(unsigned int codeSize, FILE * in,FILE * out);

void LZW_Decompress(unsigned int codeSize, FILE * in,FILE * out);

int main(int argc, char *argv[])
{
    int decompress = 0;
    FILE * in;
    FILE * out;

    unsigned int codeSize;

    codeSize = 12;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else{

        ++argv;
        --argc;

        while(1){

            if(!strcmp("--help",*argv)){
                printHelp();
                return 0;
            }
            else if(!strcmp("-d",*argv))
                decompress = 1;
/*            else if(!strcmp("-v",*argv))
              verbose = 1; */
            else if(!strncmp("-cs=",*argv,4)){
                (*argv)+= 4;

                codeSize = atoi(*argv);

                if(!(codeSize >= 9 && codeSize<=15)){
                    printf("Code length specifier is invalid, defaulting to 12.\n");
                    codeSize = 12;

                }
            } else
                break;

            ++argv;
            --argc;
        }

        if(argc < 2){
            printf("An input file AND an output file must be specified.\n");
            return 1;
        }


        in = fopen(argv[0],"rb");
        assertFileOpened(in);

        out = fopen(argv[1],"wb");
        assertFileOpened(out);

        if(decompress)
            LZW_Decompress(codeSize, in,out);
        else
            LZW_Compress(codeSize, in,out);

        fclose(in);
        fclose(out);
    }

    return 0;
}

void printHelp(void)
{
    printf("Usage: lzw IN OUT\n");
    printf("Compress or decompress a LZW encoded file.\n");
    printf("  --help\tDisplay this help message.\n");
    printf("  -c\tPerform compression. Done by default.\n");
    printf("  -d\tPerform decompression.\n");
    printf("  -cs=\tSet the outputted code sizes.\n");
}

void LZW_Compress(unsigned int codeSize, FILE * in,FILE * out)
{
    codeStr str;
    code ch;
    code nextCode;

    BitFileWriter outbits(out, MSBF);

    map<codeStr, code> stringTable;

    code maxValue = (1 << codeSize) - 1;
    code maxCode =  maxValue - 1 ;


    for(code c = 0; c < 256; ++c){
        codeStr cs;
        cs.first = c;
        cs.second = emptyCh;
        stringTable[cs] = c;
    }

    nextCode = 256;

    str.first = getc(in);
    str.second = emptyCh;

    while (true){

        ch = (code)getc(in);

        if(ch == (code)EOF)
            break;

        codeStr concat;

        concat.first = str.first;
        concat.second = ch;

        map<codeStr, code >::iterator iter =
            stringTable.find(concat);

        if(iter != stringTable.end()){
            /* if it's in the table */
            str.first = iter->second;
        } else{
            /* not in the table */

            outbits.writeBits(str.first, codeSize);

            /* if less than the maximum size */
            if(nextCode <= maxCode){

                stringTable[concat] = nextCode;

                nextCode++;
            }
            str.first = ch;
        }
        str.second = emptyCh;
    }

    outbits.writeBits(str.first, codeSize);
    outbits.writeBits(maxValue, codeSize);
    outbits.writeBits(0, codeSize);
}


void LZW_Decompress(unsigned int codeSize, FILE * in,FILE * out)
{
    code oldCode;
    code newCode;
    code nextCode;

    codeStr str;
    code ch;

    vector<codeStr> stringTable;

    BitReader inbits(in, MSBF);

    code maxValue = (1 << codeSize) - 1;
    code maxCode =  maxValue - 1 ;

    map<codeStr, vector<code> > cache;

    for(code c = 0; c < 256; ++c){
        codeStr cs;
        cs.first = c;
        cs.second = emptyCh;
        stringTable.push_back(cs);
    }

    nextCode = 256;

    oldCode = inbits.readBits(codeSize);

    putc(oldCode,out);

    ch = oldCode;

    while (true){

        newCode = inbits.readBits(codeSize);

        if(newCode == maxValue)
            break;

        /*if it is not in the translation table. */
        if(!(newCode < nextCode)){
            str.first = oldCode;
            str.second = ch;
        } else
            str = stringTable[newCode];

        ch = outputCodes(str, stringTable, cache, out);

        /* add it the table */
        if(nextCode <= maxCode){

            codeStr newEntry;

            newEntry.first = oldCode;
            newEntry.second = ch;

            stringTable.push_back(newEntry);

            nextCode++;
        }

        oldCode = newCode;
    }
}
