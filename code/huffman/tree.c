#include "tree.h"
#include <stdlib.h>

struct node * makeTreeFromTrees(struct node * tree1, struct node * tree2)
{
    struct node * tree;

    tree = malloc(sizeof(struct node));
    tree->symbol.frequency = tree1->symbol.frequency + tree2->symbol.frequency;

    /* this is not a deep copy but a shallow copy. this causes a memory corruption in
     lines 158-160*/
    tree->left = tree1;
    tree->right = tree2;

    return tree;
}

void findTwoMinValues(struct node ** trees,int length,int * tree1, int * tree2)
{
    int min1;
    int min2;
    int i,j;

    min1 = 0;
    min2 = 1;

#define freq(i) trees[i]->symbol.frequency

    for(i = 0; i < length; ++i){
	for(j = i+1; j < length; ++j){
	    if((freq(i) + freq(j)) < (freq(min1) + freq(min2))){
		min1 = i;
		min2 = j;
	    }
	}
    }

#undef freq

    *tree1 = min1;
    *tree2 = min2;
}

struct node * makeTree(BYTE symbol, unsigned long frequency)
{
    struct node * n;

    n = malloc(sizeof(struct node));
    n->symbol.symbol = symbol;
    n->symbol.frequency = frequency;

    return n;
}


void printTree(struct node * node)
{
    if(node == NULL)
	return;

    printTree(node->left);
    printAlphabetSymbol(node->symbol);
    printTree(node->right);
}
