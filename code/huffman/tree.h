#ifndef _TREE_H_
#define _TREE_H_

#include "freq_table.h"

/* A node of the Huffman tree. */
struct node {
    alphabetSymbol symbol;
    struct node * left;
    struct node * right;
};

struct node * makeTree(BYTE symbol, unsigned long frequency);

struct node * makeTreeFromTrees(struct node * tree1, struct node * tree2);

void printTree(struct node * node);

void findTwoMinValues(struct node ** trees,int length,int * tree1, int * tree2);

#endif /* _TREE_H_ */
