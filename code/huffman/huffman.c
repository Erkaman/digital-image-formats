#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common.h"
#include "huffman.h"

int main(int argc, char *argv[])
{
    FILE * in;
    FILE * out;
    char * outName;
    char * inName;
    int decompress = 0;
    char extension[5];

    verbose = 1;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else {

        /* do the command line parsing */

        /*++argv;
          --argc;
          while(1){

          if(!strcmp("--help",*argv)){
          printHelp();
          return 0;
          }
          else if(!strcmp("-d",*argv))
          decompress = 1;
          else if(!strcmp("-v",*argv))
          verbose = 1;

          ++argv;
          --argc;
          }*/

        ++argv;

        inName = *argv;
        in = fopen(inName,"rb");
        assertFileOpened(in);

        if(decompress){

            strncpyBack(extension,inName,4);

            if(!strcmp(extension,".hfm")){
                outName = changeExtension(inName,"unc");

                out = fopen(outName,"wb");
                free(outName);
                assertFileOpened(out);
            }else{

                outName = strAppend(*argv,".unc");
                out = fopen(outName,"wb");
                free(outName);
                assertFileOpened(out);
            }

            huffmanDecompress(in,out);
        }else{
            outName = strAppend(inName,".lzw");
            out = fopen(outName,"wb");
            free(outName);
            assertFileOpened(out);

            huffmanCompress(in,out);
        }

        fclose(in);
        fclose(out);
    }

    return 0;
}

void printHelp(void)
{
    printf("Usage: huffman IN\n");
    printf("Compress or decompress a Huffman coded file.\n"
           "Huffman Coding is done on a file by default");
    printf("  --help\tDisplay this help message.\n");
    printf("  -d\tPerform decompression.\n");
    printf("  -v\tVerbose output.\n");
}

void huffmanCompress(FILE * in,FILE * out)
{
    frequencyTable freqTable;
    struct node * huffmanTable;

    /* build the frequency table */
    freqTable = buildFrequencyTable(in);

    printFrequencyTable(freqTable);

    huffmanTable = constructHuffmanTree(freqTable);

    out = out;
}

struct node * constructHuffmanTree(frequencyTable freqTable)
{
    struct node * tree;
    struct node *  candidateTrees[255];
    struct node * newTree;
    int i;
    int numberOfTrees;

    int tree1;
    int tree2;

    for(i = 0; i < 255; ++i){
        candidateTrees[i] = NULL;
    }

    for(i = 0;i < freqTable.length; ++i){
        candidateTrees[i] = makeTree(
            freqTable.frequencies[i].symbol,
            freqTable.frequencies[i].frequency);

	verbosePrint("Tree %d:\n",i);
	printTree(candidateTrees[i]);
    }

    numberOfTrees = i;

    verbosePrint("numberOfTrees:%d\n",numberOfTrees);

    /* While the full Huffman tree hasn't yet been built. */
    while(numberOfTrees > 1){

	/* Find and remove the two notes with the lowest probability in the tree*/

	findTwoMinValues(candidateTrees,freqTable.length,&tree1,&tree2);

	verbosePrint("tree1:%d\n",tree1);
	verbosePrint("tree2:%d\n",tree2);

	newTree = (makeTreeFromTrees(candidateTrees[tree1],candidateTrees[tree2]));

	/* Remove these two trees */
/*	free(candidateTrees[tree1]);
	free(candidateTrees[tree2]);*/

	verbosePrint("NEW TREE:\n");
	printTree(newTree);

	candidateTrees[tree2] = newTree;
	candidateTrees[tree1] = NULL;

	/* Two of the trees were desotryed and of these two trees got replaced
	   by a new tree */
	--numberOfTrees;

        /* find the two trees with the lowest probabilities. */
    }

    /* Do Remember to free the memory taken up by the trees! */

    verbosePrint("The huffman tree:\n");

    for(i = 0; i < freqTable.length; ++i){
	if(candidateTrees[i] != NULL){
	    tree = candidateTrees[i];
	    break;
	}
    }

    printTree(tree);

    return tree;
}

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

void huffmanDecompress(FILE * in,FILE * out)
{
    in = in;
    out = out;
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
