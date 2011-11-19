#include "freq_table.h"
#include <stdlib.h>
#include <ctype.h>
#include "../common.h"

FrequencyTable buildFrequencyTable(FILE * in)
{
    FrequencyTable freqTable;
    unsigned long frequencies[256];
    int symbol;
    int i;
    int freqTablei;
    /* Start at the beginning of the file. */

    fseek(in,0,SEEK_SET);

    for(i = 0;i < 256; ++i)
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

    for(i = 0;i < 256; ++i){
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
        sizeof(AlphabetSymbol),
        alphabetSymbolCompare);

    return freqTable;
}

void printFrequencyTable(FrequencyTable freqTable)
{
    int i;

    for(i = 0;i < freqTable.length; ++i)
        printAlphabetSymbol(freqTable.frequencies[i]);
}

int alphabetSymbolCompare(const void * a, const void * b)
{
    const AlphabetSymbol * aSymbol = a;
    const AlphabetSymbol * bSymbol = b;

    if(aSymbol->frequency != bSymbol->frequency)
	return (aSymbol->frequency - bSymbol->frequency);
    else
	return (aSymbol->symbol - bSymbol->symbol);
}

void printAlphabetSymbol(AlphabetSymbol symbol)
{
    if(isprint(symbol.symbol))
        verbosePrint("printable:%c:%d\n",
                     symbol.symbol,
                     symbol.frequency);
    else
        verbosePrint("non-printable:%d:%d\n",
                     symbol.symbol,
                     symbol.frequency);
}
