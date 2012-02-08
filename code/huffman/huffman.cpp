#include "huffman.h"
#include "../io.h"

#include <algorithm>
#include <utility>
#include <cstring>
#include <vector>
#include <map>

using std::vector;
using std::sort;
using std::pair;
using std::find;
using std::sort;
using std::map;

vector<Node *> searchDepths(vector<Node *> trees, int depth);

vector<Node *> sortTrees(vector<Node *> trees);

void printTrees(vector<Node *> trees);

pair<vector<Node *>::iterator,vector<Node *>::iterator>
findTwoMinValues(vector<Node *> & trees);

CodesList createOptimumCodes(Node * huffmanTree);

vector<Node *> makeTrees(FrequencyTable freqTable);

bool nodeCmp(const Node * a, const Node * b);

bool SymbolFreqCmp(const SymbolFreq & a, const SymbolFreq & b);

int getBitToggled(BYTE value,BYTE bit);

void constructCodesList(
    const Node * node,
    BYTE depth,
    unsigned long code,
    CodesList & codes);

Node * constructHuffmanTree(FrequencyTable freqTable)
{
    Node * newTree;

    vector<Node *> trees = makeTrees(freqTable);

    /* While the full Huffman tree hasn't yet been built. */
    while(trees.size() > 1){

        /* Find the two nodes with the lowest probability in the tree*/
        pair<
            vector<Node *>::iterator,
            vector<Node *>::iterator
            > mins = findTwoMinValues(trees);

        /* These two nodes are merged into one tree.*/
        newTree = new Node(*mins.first,*mins.second);
        *mins.first = newTree;
        trees.erase(mins.second);
}

    return trees.front();
}

void printTrees(vector<Node *> trees)
{
    size_t i = 0;
    for(i = 0; i < trees.size(); ++i){
        if(trees[i] != NULL)
            trees[i]->print();
    }
}

CodesList createOptimumCodes(Node * huffmanTree)
{
    unsigned long max = huffmanTree->getMaxSymbol();

    CodesList optimumCodes(max+1);

    for(size_t i = 0; i < optimumCodes.size(); ++i){
        optimumCodes[i].codeLength = 0;
    }

    constructCodesList(huffmanTree,0,0,optimumCodes);

    return optimumCodes;
}

vector<Node *> searchDepths(vector<Node *> trees, int depth)
{
    vector<Node *> depths;

    for(vector<Node *>::iterator iter = trees.begin();
	iter != trees.end();
	++iter){
	if((*iter)->getMaxDepth() == depth)
	    depths.push_back(*iter);
    }

    return depths;
}

vector<Node *> sortTrees(vector<Node *> trees)
{
    vector<Node *> sorted;

    int depth = 1;

    while(sorted.size() != trees.size()){

	vector<Node *> depths = searchDepths(trees, depth);
	sort(depths.begin(), depths.end(), nodeCmp);
	sorted.insert(sorted.end(), depths.begin(), depths.end());
	++depth;
    }

    return sorted;
}

pair<vector<Node *>::iterator,vector<Node *>::iterator>
findTwoMinValues(vector<Node *> & trees)
{
    pair<vector<Node *>::iterator,vector<Node *>::iterator> res;

    printf("before sort\n");
    for(vector<Node *>::iterator iter = trees.begin();
	iter != trees.end();
	++iter){
	printf("S:%c D:%d F:%ld,",
	       (char)(*iter)->getMaxSymbol(),
	       (*iter)->getMaxDepth(),
	       (*iter)->getNodeValue().freq);
    }
    printf("\n");

    trees = sortTrees(trees);
/*    sort(trees.begin(), trees.end(), nodeCmp); */

    res.first = trees.begin();
    res.second = trees.begin() + 1;

    printf("after sort\n");
    for(vector<Node *>::iterator iter = trees.begin();
	iter != trees.end();
	++iter){
	printf("S:%c D:%d F:%ld,",
	       (char)(*iter)->getMaxSymbol(),
	       (*iter)->getMaxDepth(),
	       (*iter)->getNodeValue().freq);
    }
    printf("\n");

/*    printf("pair:%c;%c\n", (char)(*res.first)->getMaxSymbol(),
	(char)(*res.second)->getMaxSymbol()); */

    return res;
}

void constructCodesList(
    const Node * node,
    BYTE depth,
    unsigned long code,
    CodesList & codes)
{

    if(node->isSingle()){

        HuffmanCode newCode;
        newCode.value = code;
        newCode.codeLength = depth;

        /* Reverse the codes? */
        codes[node->getNodeValue().symbol] = newCode;

        return;
    }

    if(node->getRight() != NULL){
        constructCodesList(
            node->getRight(),
            depth+1,
            (code << 1) | (1),
            codes);
    }

    if(node->getLeft() != NULL){
        constructCodesList(
            node->getLeft(),
            depth+1,
            code << 1,
            codes);
    }
}

bool SymbolFreqCmp(const SymbolFreq & a, const SymbolFreq & b)
{
    if(a.freq != b.freq)
        return a.freq < b.freq;
    else
        return a.symbol < b.symbol;
}

vector<Node *> makeTrees(FrequencyTable freqTable)
{
    size_t i;
    vector<SymbolFreq> freqs;

    for(i = 0;i < freqTable.size(); ++i){
        if(freqTable[i] != 0){
            SymbolFreq f;
            f.symbol = i;
            f.freq = freqTable[i];

            freqs.push_back(f);
        }
    }

    sort(freqs.begin(), freqs.end(), SymbolFreqCmp);

    vector<Node *> trees;

    for(i = 0;i < freqs.size(); ++i){
        Node * node = new Node(freqs[i]);
        trees.push_back(node);
    }

    return trees;
}

void printCodesList(CodesList codes)
{
    size_t i;

    verbosePrint("Codes List:\n");

    for(i = 0; i < codes.size(); ++i)
        if(codes[i].codeLength != 0){
            HuffmanCode code = codes[i];

            verbosePrint("Code symbol:%d:%c: ",i,i);

            verbosePrint("val:%d = ",code.value);


            for(int j = (code.codeLength - 1); j >= 0 ; --j)
                verbosePrint("%d", getBitToggled(code.value,j));

            verbosePrint("\n");

        }
}

int getBitToggled(BYTE value,BYTE bit)
{
    return (((value & (1 << bit)) >> bit));
}

CodesList translateCodes(const vector<unsigned long> & codeLengths)
{
    FrequencyTable codeLengthFreqs(codeLengths);
    CodesList translatedCodes;

    unsigned long alphabetSize = codeLengths.size();

    unsigned short code;
    int bits;
    int len;

    /* Find maxBits. Find the highest possible bits length
       whose frequency is not zero. */
    int maxBits = 0;

    for(size_t i = 1;i < /*alphabetSize*/ codeLengthFreqs.size(); ++i)
        if(codeLengthFreqs[i] != 0)
            maxBits = i;

    /* enough memory? */
    vector<int> nextCode(maxBits + 1);

    code = 0;

    codeLengthFreqs.set(0,0);

    for(bits = 1; bits <= maxBits; ++bits){
        code = (code + codeLengthFreqs[bits - 1]) << 1;
        nextCode[bits] =  code;
    }

    for(size_t i = 0; i < alphabetSize; ++i){
        HuffmanCode newCode;

        newCode.value = i;
        newCode.codeLength = 0;

        translatedCodes.push_back(newCode);
    }

    for(size_t i = 0; i < codeLengths.size() /*alphabetSize*/; ++i){

        len = codeLengths[i];

        if(len != 0){
            translatedCodes[i].value = nextCode[len];
            translatedCodes[i].codeLength = len;

            nextCode[len]++;
        }
    }

    return translatedCodes;
}

bool nodeCmp(const Node * a, const Node * b)
{
    return
/*	(a->getMaxDepth() < b->getMaxDepth()) || */
        (a->getMaxSymbol() < b->getMaxSymbol()) ||
        (a->getNodeValue().freq < b->getNodeValue().freq)
        ;
}

void writeCode(HuffmanCode code, BitWriter * outBits)
{
    for(int i = 0; i < code.codeLength; ++i){
        BYTE b = getbits(
            code.value,
            code.codeLength - i - 1,
            code.codeLength - i - 1);

        outBits->writeBits(b,1);
    }
}

map<HuffmanCode, unsigned long, HuffmanCodeCompare>
reverseCodesList(const CodesList & codes)
{
    map<HuffmanCode, unsigned long, HuffmanCodeCompare> rev;

    for(size_t i = 0; i < codes.size(); ++i){
        if(codes[i].codeLength != 0){
            rev[codes[i]] = i;
        }
    }

    return rev;
}

unsigned long readCode(
    const map<HuffmanCode, unsigned long, HuffmanCodeCompare>  & codes,
    BitReader * inBits)
{
    HuffmanCode searched;

    searched.codeLength = 1;
    searched.value = inBits->readBits(1);

    map<HuffmanCode, unsigned long>::const_iterator foundIter = codes.find(searched);

    while(foundIter == codes.end()){
/*      printf("val:%ld\n", searched.value); */

        searched.codeLength++;
        searched.value = inBits->readBits(1) | (searched.value << 1);
        foundIter = codes.find(searched);
    }

    return foundIter->second;
}

