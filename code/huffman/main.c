#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common.h"
#include "main.h"
#include "tree.h"

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
                outName = changeExtension(inName,".unc");
                out = fopen(outName,"wb");
                assertFileOpened(out);
            }else{

                outName = strAppend(*argv,".unc");
                out = fopen(outName,"wb");
                assertFileOpened(out);
            }

            huffmanDecompress(in,out);

	    /* TODO: */
	    free(outName);

        }else{
            outName = strAppend(inName,".lzw");
            out = fopen(outName,"wb");
            assertFileOpened(out);

            huffmanCompress(in,out);
            free(outName);
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

/*    freqTable.length = 8;

    freqTable.frequencies[0].symbol = 'r';
    freqTable.frequencies[0].frequency = 1;

    freqTable.frequencies[1].symbol = 'p';
    freqTable.frequencies[1].frequency = 1;

    freqTable.frequencies[2].symbol = 'h';
    freqTable.frequencies[2].frequency = 1;

    freqTable.frequencies[3].symbol = 's';
    freqTable.frequencies[3].frequency = 1;

    freqTable.frequencies[4].symbol = 'e';
    freqTable.frequencies[4].frequency = 1;

    freqTable.frequencies[5].symbol = ' ';
    freqTable.frequencies[5].frequency = 2;

    freqTable.frequencies[6].symbol = 'o';
    freqTable.frequencies[6].frequency = 3;

    freqTable.frequencies[7].symbol = 'g';
    freqTable.frequencies[7].frequency = 3; */

    printFrequencyTable(freqTable);

    huffmanTable = constructHuffmanTree(freqTable);

    out = out;
}

struct node * constructHuffmanTree(frequencyTable freqTable)
{
    struct node * huffmanTree;
    struct node *  trees[256];
    struct node * newTree;
    int i;
    int numberOfTrees;

    int tree1;
    int tree2;

    verbosePrint("CONSTRUCTHUFFMANTREE\n");

    for(i = 0; i < 256; ++i)
        trees[i] = NULL;

    /* Transfer the data from the frequency table to a "wood" of nodes.  */
    for(i = 0;i < freqTable.length; ++i)
        trees[i] = makeTree(freqTable.frequencies[i]);

/*    printTrees(trees,freqTable.length);*/

    /* Always keep track of the number of trees. */
    numberOfTrees = i;

    /* While the full Huffman tree hasn't yet been built. */
    while(numberOfTrees > 1){

	/* Find and remove the two notes with the lowest probability in the tree*/

	findTwoMinValues(trees,freqTable.length,&tree1,&tree2);

/*	verbosePrint("tree1:%d\n",tree1);
	verbosePrint("tree2:%d\n",tree2);*/

	newTree = makeTreeFromTrees(
	    trees[tree1],
	    trees[tree2]);

	/* Remove these two minimum trees */
	freeTree(trees[tree1]);
	freeTree(trees[tree2]);
	trees[tree1] = NULL;
	trees[tree2] = NULL;

/*	verbosePrint("NEW TREE:\n");
	printTree(newTree);*/

	trees[tree2] = newTree;
/*	printTrees(trees,freqTable.length);*/

	/* Two of the trees were desotryed and of these two trees got replaced
	   by a new tree */
	--numberOfTrees;

    }

    /* Do remember to free the memory taken up by the trees! */

    verbosePrint("The huffman tree:\n");

    /* Find the remaining tree, this tree is the huffman tree. */
    for(i = 0; i < freqTable.length; ++i)
	if(trees[i] != NULL){
	    huffmanTree = trees[i];
	    break;
	}

    printTree(huffmanTree);

    return huffmanTree;
}


void huffmanDecompress(FILE * in,FILE * out)
{
    in = in;
    out = out;
}
