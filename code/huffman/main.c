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
            outName = strAppend(inName,".hfm");
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
    FrequencyTable freqTable;
    CodesList optimumCodes;
    struct Node * huffmanTree;

/*    freqTable = buildFrequencyTable(in);
*/

    freqTable.length = 4;

    in = in;

    freqTable.frequencies[0].symbol = 'e';
    freqTable.frequencies[0].frequency = 1;

    freqTable.frequencies[1].symbol = 'r';
    freqTable.frequencies[1].frequency = 1;

    freqTable.frequencies[2].symbol = 'i';
    freqTable.frequencies[2].frequency = 1;

    freqTable.frequencies[3].symbol = 'c';
    freqTable.frequencies[3].frequency = 1;

    printFrequencyTable(freqTable);

    huffmanTree = constructHuffmanTree(freqTable);

    optimumCodes = createOptimumCodes(huffmanTree);

/*    huffmanEncode(in,out,optimumCodes);*/

    out = out;

}

struct Node * constructHuffmanTree(FrequencyTable freqTable)
{
    struct Node * huffmanTree;
    struct Node *  trees[256];
    struct Node * newTree;
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

/*      verbosePrint("tree1:%d\n",tree1);
        verbosePrint("tree2:%d\n",tree2);*/

        newTree = makeTreeFromTrees(
            trees[tree1],
            trees[tree2]);

        /* Remove these two minimum trees */
        freeTree(trees[tree1]);
        freeTree(trees[tree2]);
        trees[tree1] = NULL;
        trees[tree2] = NULL;

/*      verbosePrint("NEW TREE:\n");
        printTree(newTree);*/

        trees[tree2] = newTree;
/*      printTrees(trees,freqTable.length);*/

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

CodesList createOptimumCodes(struct Node * huffmanTree)
{
    CodesList optimumCodes;

    verbosePrint("createOptimumCodes\n");

    optimumCodes = makeNewCodesList();
    traverseHuffmanTree(huffmanTree,0,0,&optimumCodes);

    printCodesList(optimumCodes);

    return optimumCodes;
}

CodesList makeNewCodesList(void)
{
    int i = 0;
    CodesList codes;

    for(i = 0; i < 256; ++i)
	codes.codes[i].codeLength = 0;

    return codes;
}


void huffmanDecompress(FILE * in,FILE * out)
{
    in = in;
    out = out;
}

void traverseHuffmanTree(
    struct Node * node,
    BYTE depth,
    BYTE code,
    CodesList * codes)
{
    Code newCode;

    /* bit shifting */

/*    verbosePrint("traverseHuffmanTree\n");
    verbosePrint("depth:%d\n",depth);
    verbosePrint("code:%d\n",code);*/

    if(isEmptyNode(node)){

        newCode.symbol = node->symbol.symbol;
        newCode.value = code;
        newCode.codeLength = depth;

        codes->codes[node->symbol.symbol] = newCode;

        return;
    }

    if(node->left != NULL){
        traverseHuffmanTree(
            node->left,
            depth+1,
            code | (1 << depth),
            codes);
    }

    if(node->right != NULL){
        traverseHuffmanTree(
            node->right,
            depth+1,
            code,
            codes);
    }
}

void printCode(Code code)
{
    int i;

    verbosePrint("Code symbol:%d:%c: ",code.symbol,code.symbol);

    verbosePrint("val:%d = ",code.value);

    for(i = (code.codeLength - 1); i >= 0 ; --i)
        verbosePrint("%d", getBitToggled(code.value,i));

    verbosePrint("\n");
}

int getBitToggled(BYTE value,BYTE bit)
{
    return (((value & (1 << bit)) >> bit));
}

void printCodesList(CodesList codes)
{
    int i;

    verbosePrint("Codes List:\n");

    for(i = 0; i < 256; ++i)
	if(codes.codes[i].codeLength != 0)
	    printCode(codes.codes[i]);
}

void huffmanEncode(FILE * in,FILE * out, CodesList codes)
{
    int c;
    int sum;

    verbosePrint("Huffman encode\n");

    /* start at the beginning of the file */
    fseek(in,0,SEEK_SET);

    sum = 0;

    c = getc(in);
    while(c != EOF){

	printCode(codes.codes[c]);
	outputCode(codes.codes[c],out);
	sum += codes.codes[c].codeLength;
	c = getc(in);
    }

    verbosePrint("Code length:%d\n",sum);

     out = out;
}

void outputCode(Code c,FILE * out)
{
/*    The number of remaining bits to be written before a whole new byte is outputted. */
    static int remainingPacketBits = 8;
    static BYTE packet = 0;
    static int shift = 8;

    int code = c.value;

/*    the number of remaing bits in the current code. */
    int remainingCodeBits;

/*    verbosePrint("code:%d\n",code); */

    remainingCodeBits = c.codeLength;

    /* while all bits haven't yet been read. */
    while(remainingCodeBits > 0){

	/* if the number of bits to be read is less than the current input value */
        if(remainingPacketBits < remainingCodeBits){
            /* write what can be written*/

/*	    verbosePrint("B1\n"); */

	    shift -= remainingPacketBits;
            packet |=
                ((code >> (remainingCodeBits - remainingPacketBits)) << shift);

/*	    verbosePrint("output packet:%x\n",packet); */

            putc(packet,out);

            remainingCodeBits -= remainingPacketBits;
/*	    verbosePrint("remainingCodeBits:%d\n",remainingCodeBits);
	    verbosePrint("remainingPacketBits:%d\n",remainingPacketBits);*/

	    /* get rid of n last bits */
	    code <<= c.codeLength - remainingPacketBits;
	    code >>= c.codeLength - remainingPacketBits;

	    /* reset the packet */
	    remainingPacketBits = 8;
	    packet = 0;
            shift = 8;
        }else{
	    /* if the number of bits remaining to be read is less than the
	     bits in the current input value. */

            /* remainingPacketBits >= remainingCodeBits */
/*	    code = remainingCodeBits;*/

/*	    verbosePrint("B2\n"); */

            shift -= remainingCodeBits;

/*	    verbosePrint("shift:%d\n",shift); */
/*	    verbosePrint("B1:%d\n",code << shift); */

	    packet |= code << shift;
/*	    verbosePrint("packet:%x\n",packet); */

            remainingPacketBits -=  remainingCodeBits;

            remainingCodeBits = 0;
        }

    }
}