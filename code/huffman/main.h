#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#include "freq_table.h"
#include "tree.h"

void printHelp(void);

void huffmanCompress(FILE * in,FILE * out);
void huffmanDecompress(FILE * in,FILE * out);

struct node * constructHuffmanTree(frequencyTable freqTable);


#endif /* _HUFFMAN_H_ */
