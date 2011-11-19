#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#include "freq_table.h"
#include "tree.h"

typedef struct{
    /* the code value of the code*/
    BYTE value;
    BYTE symbol;

    /* maxvalue: 256 */
    /* The codes whose lengths are zero are ignored.  */
    int codeLength;
} Code;

typedef struct{
    /* the codes are used as indices. */
    Code codes[256];
} CodesList;

void printHelp(void);

void huffmanCompress(FILE * in,FILE * out);
void huffmanDecompress(FILE * in,FILE * out);

struct Node * constructHuffmanTree(FrequencyTable freqTable);

CodesList createOptimumCodes(struct Node * huffmanTree);

void printCode(Code code);

void traverseHuffmanTree(
    struct Node * node,
    BYTE depth,
    BYTE code,
    CodesList * codes);

int getBitToggled(BYTE value,BYTE bit);

void printCodesList(CodesList codes);

void huffmanEncode(FILE * in,FILE * out, CodesList codes);

CodesList makeNewCodesList(void);

void outputCode(Code c,FILE * out);

#endif /* _HUFFMAN_H_ */
