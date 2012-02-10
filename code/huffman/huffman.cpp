#include "huffman.h"
#include "../io.h"

#include <algorithm>
#include <utility>
#include <cstring>
#include <vector>
#include <map>
#include <cassert>

using std::vector;
using std::sort;
using std::pair;
using std::find;
using std::sort;
using std::map;
using std::fill;
using std::min;

typedef unsigned int Symbol;
typedef unsigned int Freq;
typedef unsigned int CodeLength;

struct SymbolFreq{
public:
    Symbol symbol;
    Freq freq;
};

bool symbolFreqCmp(const SymbolFreq & a, const SymbolFreq & b);

/* A node of the Huffman tree. */
struct Node {

public:
    SymbolFreq val;

    static const Symbol emptySymbol = 0;

    /* Values of nodes should not change, just move them around*/
    Node * left;
    Node * right;

    Node(SymbolFreq val_):val(val_), left(NULL), right(NULL){}

    Node(Node * left_, Node * right_):left(left_), right(right_)
        {
            this->val.symbol = emptySymbol;
            this->val.freq =
                this->left->val.freq + this->right->val.freq;
        }

    bool isSingle()const
        {
            return (this->left == NULL && this->right == NULL);
        }
};

Node * constructHuffmanTree(FrequencyTable freqTable,unsigned long & len);

void printTrees(vector<Node *> trees);

pair<vector<Node *>::iterator,vector<Node *>::iterator>
findTwoMinValues(vector<Node *> & trees);

vector<Node *> makeTrees(const FrequencyTable & freqTable);

bool nodeCmp(const Node * a, const Node * b);

void getCodeDepths(int depth, Node * node, vector<unsigned int> & codeDepths);

vector<unsigned int> getCodeDepths(
    FrequencyTable freqTable,
    Node * node);

int getBitToggled(BYTE value,BYTE bit);

void getCodeLengths(
    const Node * node,
    /* A symbol value to a code length.*/
    vector<unsigned long> & codeLengths,
    int depth);

Node * constructHuffmanTree(FrequencyTable freqTable, unsigned long & len)
{

    vector<Node *> trees = makeTrees(freqTable);
    len = trees.size();

    /* While the full Huffman tree hasn't yet been built. */
    while(trees.size() > 1){

        /* Find the two nodes with the lowest probability in the tree*/
        pair<
            vector<Node *>::iterator,
            vector<Node *>::iterator
            > mins = findTwoMinValues(trees);

        Node * newTree;

        /* These two nodes are merged into one tree.*/
        newTree = new Node(*mins.first,*mins.second);
        *mins.first = newTree;
        trees.erase(mins.second);
    }
    return trees.front();
}

void getCodeLengths(
    const Node * node,
    /* A symbol value to a code length.*/
    vector<unsigned long> & codeLengths,
    int depth)
{
    if(node->isSingle()){
        codeLengths[node->val.symbol] = depth;
/*      codeLengths.push_back(depth); */
        return;
    }

    if(node->right != NULL)
        getCodeLengths(
            node->right,
            codeLengths,
            depth+1);

    if(node->left != NULL)
        getCodeLengths(
            node->left,
            codeLengths,
            depth+1);

}

vector<unsigned int> makeCodeLengths(
    /* all the codes to be used should be assigned freqs, even the ones with no freqs at all*/
    FrequencyTable freqTable,
    CodeLength maxCodeLength)
{
    unsigned long len;
    Node * huffman = constructHuffmanTree(freqTable,len);

    vector<unsigned int> codeDepths = getCodeDepths(freqTable, huffman);

    vector<unsigned long> blCount(maxCodeLength+1);

    unsigned int sum = 0;

    /* Count the frequencies of the code depths. */
    for(size_t i = 0; i < codeDepths.size(); ++i){
        if(codeDepths[i] != 0){
            size_t depth = min(maxCodeLength,codeDepths[i]);
	    printf("depth:%ld\n", depth);
            blCount[depth]++;
	    sum += 1 << (maxCodeLength - depth);
        }
    }

    /* HERE BE DRAGONS */
    unsigned int overflow = sum > (unsigned int)(1 << maxCodeLength) ? sum - (1 << maxCodeLength) : 0;

    while (overflow--)
    {
        unsigned int bits = maxCodeLength-1;
        while (blCount[bits] == 0)
            bits--;
        blCount[bits]--;
        blCount[bits+1] += 2;
        assert(blCount[maxCodeLength] > 0);
        blCount[maxCodeLength]--;
    }

    vector<SymbolFreq> symbols;

    for(size_t i = 0; i < freqTable.size(); ++i){

	SymbolFreq s;

	s.symbol = i;
	s.freq = freqTable[i];

	symbols.push_back(s);
    }

    sort(symbols.begin(), symbols.end(), symbolFreqCmp);

    SymbolFreq zeroFreqSymbol;
    zeroFreqSymbol.freq = 0;

    size_t beginCodeLengths =
	upper_bound(symbols.begin(), symbols.end(), zeroFreqSymbol, symbolFreqCmp) -
        symbols.begin();

    vector<unsigned int> codeLengths(freqTable.size());

    for(size_t i = 0; i < beginCodeLengths; ++i)
	codeLengths[i] = 0;

    unsigned int bits = maxCodeLength;
    for(size_t i = beginCodeLengths; i < symbols.size(); ++i){
        while (blCount[bits] == 0)
            bits--;
        codeLengths[symbols[i].symbol] = bits;
        blCount[bits]--;
    }

    return codeLengths;
}

pair<vector<Node *>::iterator,vector<Node *>::iterator>
findTwoMinValues(vector<Node *> & trees)
{
    pair<vector<Node *>::iterator,vector<Node *>::iterator> res;

    sort(trees.begin(), trees.end(), nodeCmp);

    res.first = trees.begin();
    res.second = trees.begin() + 1;

    return res;
}

vector<Node *> makeTrees(const FrequencyTable & freqTable)
{
    size_t i;
    vector<Node *> trees;

    for(i = 0;i < freqTable.size(); ++i){
        if(freqTable[i] != 0){
            SymbolFreq f;
            f.symbol = i;
            f.freq = freqTable[i];
            Node * node = new Node(f);
            trees.push_back(node);
        }
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

CodesList translateCodes(const vector<unsigned int> & codeLengths)
{
    FrequencyTable codeLengthFreqs = constructFrequencyTable(codeLengths, 256);
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

    codeLengthFreqs[0] = 0;

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
    return (a->val.freq < b->val.freq);
}

void writeCode(HuffmanCode code, BitWriter * outBits)
{
    for(CodeLength i = 0; i < code.codeLength; ++i){
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

void getCodeDepths(int depth, Node * node, vector<unsigned int> & codeDepths)
{
    if(node->isSingle())
        codeDepths[node->val.symbol] = depth;
    else{
        if(node->left != NULL)
            getCodeDepths(depth+1, node->left, codeDepths);

        if(node->right != NULL)
            getCodeDepths(depth+1, node->right, codeDepths);
    }
}

vector<unsigned int> getCodeDepths(
    FrequencyTable freqTable,
    Node * node)
{
    size_t maxSymbol = freqTable.end() - 1 - freqTable.begin();

    vector<unsigned int> codeDepths(maxSymbol + 1);
    fill(codeDepths.begin(), codeDepths.end(), 0);

    getCodeDepths(0, node, codeDepths);

    return codeDepths;
}

bool symbolFreqCmp(const SymbolFreq & a, const SymbolFreq & b)
{
    return a.freq < b.freq;
}
