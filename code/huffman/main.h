#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#include "freq_table.h"

void printHelp(void);

void huffmanCompress(FILE * in,FILE * out);
void huffmanDecompress(FILE * in,FILE * out);



/* A node of the Huffman tree. */
struct node {
    alphabetSymbol symbol;
    struct node * left;
    struct node * right;
};

struct node * makeTree(BYTE symbol, unsigned long frequency);

struct node * makeTreeFromTrees(struct node * tree1, struct node * tree2);

void printTree(struct node * node);

struct node * constructHuffmanTree(frequencyTable freqTable);

void findTwoMinValues(struct node ** trees,int length,int * tree1, int * tree2);

#endif /* _HUFFMAN_H_ */
