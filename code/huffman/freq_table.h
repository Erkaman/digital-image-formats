#ifndef _FREQ_TABLE_H_
#define _FREQ_TABLE_H_

#include "../common.h"

typedef struct{
    BYTE symbol;
    unsigned long frequency;
}  alphabetSymbol;


typedef struct{
    alphabetSymbol frequencies[255];
    BYTE length;
} frequencyTable;


frequencyTable buildFrequencyTable(FILE * in);

void printFrequencyTable(frequencyTable freqTable);

int alphabetSymbolCompare(const void * a, const void * b);

void printAlphabetSymbol(alphabetSymbol symbol);

#endif /* _FREQ_TABLE_H_ */
