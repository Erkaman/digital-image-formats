#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "../common.h"
#include "huffman.h"

int verbose;

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;
    char * outName;
    char * inName;
    int decompress = 0;
    char extension[5];

    verbose = 1;



    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else {

        /* do the command line parsing */

        /*++argv;
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

          ++argv;
          --argc;
          }*/

        ++argv;

        inName = *argv;
        in = fopen(inName,"rb");
        assertFileOpened(in);

        if(decompress){

            strncpyBack(extension,inName,4);

            if(!strcmp(extension,".hfm")){
                outName = changeExtension(inName,"unc");

                out = fopen(outName,"wb");
                free(outName);
                assertFileOpened(out);
            }else{

                outName = strAppend(*argv,".unc");
                out = fopen(outName,"wb");
                free(outName);
                assertFileOpened(out);
            }

            huffmanDecompress(in,out);
        }else{
            outName = strAppend(inName,".lzw");
            out = fopen(outName,"wb");
            free(outName);
            assertFileOpened(out);

            huffmanCompress(in,out);
        }

        fclose(in);
        fclose(out);
    }

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
    frequencyTable freqTable;

    /* build the frequency table */
    freqTable = buildFrequencyTable(in);

    printFrequencyTable(freqTable);

    in = in;
    out = out;
}

void huffmanDecompress(FILE * in,FILE * out)
{
    in = in;
    out = out;
}

frequencyTable buildFrequencyTable(FILE * in)
{
    frequencyTable freqTable;
    unsigned long frequencies[255];
    int symbol;
    int i;
    int freqTablei;
    /* Start at the beginning of the file. */

    fseek(in,0,SEEK_SET);

    for(i = 0;i < 255; ++i)
        frequencies[i] = 0;

    /* find the freqencies of all the possible values a byte can have.
       In other words; find the frequency of all the symbols in the file.*/

    symbol = getc(in);
    while(symbol != EOF){

        ++frequencies[symbol];

        symbol = getc(in);
    }

    /* Organize all the frequencies into a neat frequency table. */

    freqTable.length = 0;
    freqTablei = 0;

    for(i = 0;i < 255; ++i){
        /* If the symbol occurred at all in the file*/
        if(frequencies[i] != 0){

            /* Put the symbol and its information into the frequency table */

            freqTable.frequencies[freqTablei].symbol = i;
            freqTable.frequencies[freqTablei].frequency = frequencies[i];

            ++freqTablei;
            ++freqTable.length;
        }
    }

    /* return to the beginning of the file once the frequency table has been
       build */
    fseek(in,0,SEEK_SET);

    /* Sort the frequency table in ascending order. */

    qsort(
        freqTable.frequencies,
        freqTable.length,
        sizeof(alphabetSymbol),
        alphabetSymbolCompare);

    return freqTable;
}

int alphabetSymbolCompare(const void * a, const void * b)
{
    const alphabetSymbol * aSymbol = a;
    const alphabetSymbol * bSymbol = b;

    return (aSymbol->frequency - bSymbol->frequency);
}

void verbosePrint(const char * format, ...)
{
    va_list vl;

    if(verbose){
        va_start(vl, format);
        vprintf(format, vl);
        va_end(vl);
    }
}

void printFrequencyTable(frequencyTable freqTable)
{
    int i;

    verbosePrint("Printing Frequency Table:\n");

    for(i = 0;i < freqTable.length; ++i){
        if(isprint(freqTable.frequencies[i].symbol))
            verbosePrint("printable:%c:%d\n",
                         freqTable.frequencies[i].symbol,
                         freqTable.frequencies[i].frequency);
        else
            verbosePrint("non-printable:%c:%d\n",
                         freqTable.frequencies[i].symbol,
                         freqTable.frequencies[i].frequency);
    }
}
