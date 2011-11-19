#include "tree.h"
#include <stdlib.h>

struct Node * makeTreeFromTrees(struct Node * tree1, struct Node * tree2)
{
    struct Node * tree;

    tree = (struct Node *)malloc(sizeof(struct Node));
    tree->symbol.frequency = tree1->symbol.frequency + tree2->symbol.frequency;
    tree->symbol.symbol = 0;

    tree->left = deepCopyTree(tree1);
    tree->right = deepCopyTree(tree2);

    return tree;
}

void findTwoMinValues(struct Node ** trees,int length,int * tree1, int * tree2)
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

struct Node * makeTree(AlphabetSymbol symbol)
{
    struct Node * n;

    n = (struct Node *)malloc(sizeof(struct Node));
    n->symbol = symbol;

    return n;
}

void printTree(struct Node * node)
{
    if(node == NULL)
        return;

    printTree(node->left);
    printAlphabetSymbol(node->symbol);
    printTree(node->right);
}

struct Node * deepCopyTree(struct Node * tree)
{
    struct Node * copy;

    copy = makeTree(tree->symbol);

    if(tree->left != NULL)
        copy->left = deepCopyTree(tree->left);

    if(tree->right != NULL)
        copy->right = deepCopyTree(tree->right);

    return copy;
}

void freeTree(struct Node * tree)
{
    if(tree->left != NULL)
        freeTree(tree->left);

    if(tree->right != NULL)
        freeTree(tree->right);

    free(tree);
    tree = NULL;
}

void printTrees(struct Node ** trees,int length)
{
    int i = 0;
    for(i = 0; i < length; ++i){
        if(trees[i] != NULL){}
            printTree(trees[i]);
    }
}

int isEmptyNode(struct Node * node)
{
    return (node->left == NULL && node->right == NULL);
}


int maxDepth(struct Node * node)
{
    int lDepth;
    int rDepth;

    if (node==NULL)
        return 0;
    else {
        lDepth = maxDepth(node->left);
        rDepth = maxDepth(node->right);

        if (lDepth > rDepth)
            return(lDepth+1);
        else
            return(rDepth+1);
    }
}

/*void printTree(struct Node * node, int level)
{
    if(node == NULL) {
        padding ('\t', level);
        puts ( "~" );
    }
    else {
        printTree(node->right, level + 1);
        padding('\t', level);
        verbosePrint("%c\n", node->symbol.symbol);
        printTree(node->left, level + 1);
    }
}

void padding(char ch, int n)
{
    int i;

    for(i = 0; i < n; i++ )
        verbosePrint("%c",ch);
}*/
