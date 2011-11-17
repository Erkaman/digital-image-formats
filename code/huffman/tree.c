#include "tree.h"
#include <stdlib.h>

struct node * makeTreeFromTrees(struct node * tree1, struct node * tree2)
{
    struct node * tree;

    tree = (struct node *)malloc(sizeof(struct node));
    tree->symbol.frequency = tree1->symbol.frequency + tree2->symbol.frequency;
    tree->symbol.symbol = 0;
    /* this is not a deep copy but a shallow copy. this causes a memory corruption in
     lines 158-160*/
    tree->left = deepCopyTree(tree1);
    tree->right = deepCopyTree(tree2);

    return tree;
}

void findTwoMinValues(struct node ** trees,int length,int * tree1, int * tree2)
{
    int min1;
    int min2;
    int i,j;

    for(i = 0;i < length; ++i)
	if(trees[i] != NULL)
	    break;

    min1 = i;

    for(j = i+1;j < length; ++j)
	if(trees[j] != NULL)
	    break;

    min2 = j;

#define freq(i) trees[i]->symbol.frequency

    for(i = 0; i < length; ++i){

	if(trees[i] == NULL)
	    continue;

	for(j = i+1; j < length; ++j){

	    if(trees[j] == NULL)
		continue;

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

struct node * makeTree(alphabetSymbol symbol)
{
    struct node * n;

    n = (struct node *)malloc(sizeof(struct node));
    n->symbol = symbol;

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

struct node * deepCopyTree(struct node * tree)
{
    struct node * copy;

    copy = makeTree(tree->symbol);

    if(tree->left != NULL)
	copy->left = deepCopyTree(tree->left);

    if(tree->right != NULL)
	copy->right = deepCopyTree(tree->right);

    return copy;
}

void freeTree(struct node * tree)
{
    if(tree->left != NULL)
	freeTree(tree->left);

    if(tree->right != NULL)
	freeTree(tree->right);

    free(tree);
    tree = NULL;
}

void printTrees(struct node ** trees,int length)
{
    int i = 0;
    for(i = 0; i < length; ++i){
	if(trees[i] != NULL)
	    printTree(trees[i]);
    }
}


