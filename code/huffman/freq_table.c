#include "freq_table.h"
#include <stdlib.h>
#include <ctype.h>
#include "debug.h"

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

void printFrequencyTable(frequencyTable freqTable)
{
    int i;

    verbosePrint("Printing Frequency Table:\n");

    for(i = 0;i < freqTable.length; ++i){
        printAlphabetSymbol(freqTable.frequencies[i]);
    }
}

int alphabetSymbolCompare(const void * a, const void * b)
{
    const alphabetSymbol * aSymbol = a;
    const alphabetSymbol * bSymbol = b;

    return (aSymbol->frequency - bSymbol->frequency);
}

void printAlphabetSymbol(alphabetSymbol symbol)
{
    if(isprint(symbol.symbol))
        verbosePrint("printable:%c:%d\n",
                     symbol.symbol,
                     symbol.frequency);
    else
        verbosePrint("non-printable:%c:%d\n",
                     symbol.symbol,
                     symbol.frequency);
}

