#ifndef _TREE_H_
#define _TREE_H_

#include "freq_table.h"

/* A node of the Huffman tree. */
struct Node {
    AlphabetSymbol symbol;
    struct Node * left;
    struct Node * right;
};

struct Node * makeTree(AlphabetSymbol symbol);

struct Node * makeTreeFromTrees(struct Node * leftTree, struct Node * rightTree);

void findTwoMinValues(struct Node ** trees,int length,int * tree1, int * tree2);

struct Node * deepCopyTree(struct Node * tree);

void freeTree(struct Node * tree);

void printTrees(struct Node ** trees,int length);

int isEmptyNode(struct Node * node);

int maxDepth(struct Node * node);

void printTree(struct Node * node);

/*void padding(char ch, int n);

void printTree(struct Node * node, int level);*/

#endif /* _TREE_H_ */
