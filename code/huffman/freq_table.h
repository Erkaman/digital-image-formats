#ifndef _FREQ_TABLE_H_
#define _FREQ_TABLE_H_

#include "../common.h"

typedef struct{
    BYTE symbol;
    unsigned long frequency;
}  AlphabetSymbol;


typedef struct{
    AlphabetSymbol frequencies[255];
    BYTE length;
} FrequencyTable;


FrequencyTable buildFrequencyTable(FILE * in);

void printFrequencyTable(FrequencyTable freqTable);

int alphabetSymbolCompare(const void * a, const void * b);

void printAlphabetSymbol(AlphabetSymbol symbol);

#endif /* _FREQ_TABLE_H_ */
