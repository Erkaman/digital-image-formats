#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

void printHelp(void);

void huffmanCompress(FILE * in,FILE * out);
void huffmanDecompress(FILE * in,FILE * out);
void verbosePrint(const char * format, ...);

typedef struct{
    BYTE symbol;
    unsigned long frequency;
}  alphabetSymbol;

typedef struct{
    alphabetSymbol frequencies[255];
    BYTE length;
} frequencyTable;

int alphabetSymbolCompare(const void * a, const void * b);

frequencyTable buildFrequencyTable(FILE * in);

void printFrequencyTable(frequencyTable freqTable);

frequencyTable makeNewFrequencyTable(void);

/* binary tree */
typedef struct {
    unsigned long data;
    struct node* left;
    struct node* right;
} node;

#endif /* _HUFFMAN_H_ */
