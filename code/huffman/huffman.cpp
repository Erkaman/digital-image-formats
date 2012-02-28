#include "huffman.h"
#include "../io.h"

#include <algorithm>
#include <utility>
#include <cstring>
#include <vector>
#include <map>
#include <cassert>
#include <iostream>
#include <iterator>

#include "node.h"

using std::vector;
using std::sort;
using std::pair;
using std::find;
using std::sort;
using std::map;
using std::fill;
using std::min;
using std::ostream_iterator;
using std::cout;

vector<BYTE> repeatCode(
    unsigned int minCodeLength,
    unsigned int extraBits,
    unsigned int repeatCode,
    BitReader * compressedStream);

Node * constructHuffmanTree(FrequencyTable freqTable,unsigned long & len);

void printTrees(vector<Node *> trees);

CodeLengths makeCodeLengths(
    FrequencyTable freqTable,
    CodeLength maxCodeLength);

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

CodeLengths makeCodeLengths(
    /* all the codes to be used should be assigned freqs, even the ones with no freqs at all*/
    FrequencyTable freqTable,
    CodeLength maxCodeLength)
{
    unsigned long len;

    /* first a normal, condition-less Huffman tree is constructed*/
    Node * huffman = constructHuffmanTree(freqTable,len);

    /* Get the code depths of all the codes of the condition-less Huffman tree*/
    vector<unsigned int> codeDepths = getCodeDepths(freqTable, huffman);

    delete huffman;

    /* contains the frequencies of the different code lengths */
    vector<unsigned long> blCount(maxCodeLength+1);

    unsigned int sum = 0;

    /* Count the frequencies of the code depths. */
    for(size_t i = 0; i < codeDepths.size(); ++i){
        if(codeDepths[i] != 0){
            size_t depth = min(maxCodeLength,codeDepths[i]);
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

    /* Sort the symbols by increasing frequency */

    vector<SymbolFreq> symbols;

    /* the table is went through in order. Therefore the symbols will
     * also get sorted by order. The order of the separate code length groups will be maintained in the sorting process.*/
    for(size_t i = 0; i < freqTable.size(); ++i){

        SymbolFreq s;

        s.symbol = i;
        s.freq = freqTable[i];

        symbols.push_back(s);
    }

    sort(symbols.begin(), symbols.end(), symbolFreqCmp);

    /* */

    SymbolFreq zeroFreqSymbol;
    zeroFreqSymbol.freq = 0;

    /* The code are sorted by increasing frequency,
     so find the beginning of the non-zero frequency codes
    at the end of the array*/
    size_t beginCodeLengths =
        upper_bound(symbols.begin(), symbols.end(), zeroFreqSymbol, symbolFreqCmp) -
        symbols.begin();

    CodeLengths codeLengths(freqTable.size());

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
        if(codes[i].length != 0){
            HuffmanCode code = codes[i];

            verbosePrint("Code symbol:%d:%c: ",i,i);

            verbosePrint("val:%d = ",code.value);


            for(int j = (code.length - 1); j >= 0 ; --j)
                verbosePrint("%d", getBitToggled(code.value,j));

            verbosePrint("\n");

        }
}

int getBitToggled(BYTE value,BYTE bit)
{
    return (((value & (1 << bit)) >> bit));
}

CodesList translateCodes(const CodeLengths & codeLengths)
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
        newCode.length = 0;

        translatedCodes.push_back(newCode);
    }


    for(size_t i = 0; i < codeLengths.size() /*alphabetSize*/; ++i){

        len = codeLengths[i];

        if(len != 0){
            translatedCodes[i].value = nextCode[len];
            translatedCodes[i].length = len;

            nextCode[len]++;
        }
    }


    printf("codes list\n");
    printCodesList(translatedCodes);

    return translatedCodes;
}

bool nodeCmp(const Node * a, const Node * b)
{
    return (a->val.freq < b->val.freq);
}

void writeCode(HuffmanCode code, BitWriter * outBits)
{
    for(CodeLength i = 0; i < code.length; ++i){
        BYTE b = getbits(
            code.value,
            code.length - i - 1,
            code.length - i - 1);

        outBits->writeBits(b,1);
    }
}

map<HuffmanCode, unsigned long, HuffmanCodeCompare>
reverseCodesList(const CodesList & codes)
{
    map<HuffmanCode, unsigned long, HuffmanCodeCompare> rev;

    for(size_t i = 0; i < codes.size(); ++i){
        if(codes[i].length != 0){
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

    searched.length = 1;
    searched.value = inBits->readBits(1);

    map<HuffmanCode, unsigned long>::const_iterator foundIter = codes.find(searched);

    while(foundIter == codes.end()){
        searched.length++;
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
    /* get size of alphabet. */
    size_t maxSymbol = freqTable.end() - 1 - freqTable.begin();

    /* Initialize all the code depths to that of empty codes. */
    vector<unsigned int> codeDepths(maxSymbol + 1);
    fill(codeDepths.begin(), codeDepths.end(), 0);

    /* Get the code depths */
    getCodeDepths(0, node, codeDepths);

    return codeDepths;
}

RevCodesList loadCodeLengthCodes(unsigned int HCLEN,BitReader * compressedStream)
{
    unsigned int  realLength;
    BYTE codeLengthOrder[CODE_LENGTH_CODES] = {
        16, 17, 18,0, 8, 7,
        9, 6, 10, 5, 11, 4,
        12, 3, 13, 2, 14, 1, 15};

    CodeLengths codeLengths(CODE_LENGTH_CODES);

    unsigned int i;

    realLength = HCLEN + 4;

    for(i = 0; i < CODE_LENGTH_CODES; ++i)
        codeLengths[i] = 0;

    for(i = 0; i < realLength; ++i){
        BYTE b  = compressedStream->readBits(3);
        codeLengths[codeLengthOrder[i]] = b;
    }

    return reverseCodesList(translateCodes(codeLengths));
}

vector<BYTE> repeatCode(
    unsigned int minCodeLength,
    unsigned int extraBits,
    unsigned int repeatCode,
    BitReader * compressedStream)
{
    int realLength;
    int j;

    vector<BYTE> rep;

    realLength =  minCodeLength + compressedStream->readBits(extraBits);

    for(j = 0; j < realLength; ++j)
        rep.push_back(repeatCode);

    return rep;
}

vector<BYTE> repeatZeroLengthCode(
    unsigned int minCodeLength,
    unsigned int extraBits,
    BitReader * compressedStream){
    return repeatCode(minCodeLength, extraBits, 0, compressedStream);
}

vector<BYTE> repeatPreviousLengthCode(
    unsigned int previousCode,
    BitReader * compressedStream){
    return repeatCode(3, 2, previousCode, compressedStream);
}


CodeLengths compressCodeLengths(
    const CodeLengths & codeLengths,
    CodeLengths & codeLengthsAlphabetCodeLengths)
{
    CodeLengths compressed;

    CodeLengths::const_iterator nextFound, found = codeLengths.begin();

    nextFound = find_first_not_of(found, codeLengths.end(), *found);

    if(codeLengths.size() == 0)
        return compressed;

    do{

        /* write packet. */
        unsigned int packetLength = nextFound - found;

        /* does this decrease compression performance? */
        if(packetLength < 2){
            compressed.push_back(*found);
            codeLengthsAlphabetCodeLengths.push_back(*found);
        } else {

            CodeLengths packet = makePacket(
                packetLength,
                *found,
                codeLengthsAlphabetCodeLengths);

            compressed.insert(
                compressed.end(),
                packet.begin(),
                packet.end());
        }


        found = nextFound;
        nextFound = find_first_not_of(found, codeLengths.end(), *found);
    } while(found != codeLengths.end());

    return compressed;
}

CodeLengths makePacket(
    CodeLength length,
    unsigned int code,
    CodeLengths & codeLengthsAlphabetCodeLengths)
{
    if(code == 0){
        return makeZeroPacket(length,codeLengthsAlphabetCodeLengths);
    } else {
        return makeRepeatPacket(length, code, codeLengthsAlphabetCodeLengths);
    }
}

CodeLengths makeRepeatPacket(
    CodeLength length,
    unsigned int code,
    CodeLengths & codeLengthsAlphabetCodeLengths)
{
    CodeLengths packet;

    packet.push_back(code);
    codeLengthsAlphabetCodeLengths.push_back(code);

    --length;

    while(length > 0){

        if(length > 2){
            packet.push_back(16);
            codeLengthsAlphabetCodeLengths.push_back(16);

            unsigned int packetLength;

            packetLength = min((unsigned int)6, length);

            length -=  packetLength;
            packetLength -= 3;
            packet.push_back(packetLength);
        } else {
            for(unsigned int i = 0; i < length; ++i){
                packet.push_back(code);
                codeLengthsAlphabetCodeLengths.push_back(code);
            }
            length = 0;
        }
    }

    return packet;
}

CodeLengths makeZeroPacket(
    CodeLength length,
    CodeLengths & codeLengthsAlphabetCodeLengths)
{
    CodeLengths packet;

    while(length > 0){

        /* Even runs of three zeroes can efficiently be encoded to 2 codes.*/
        if(length > 2){

            unsigned int packetLength;

            if(length >= 11){
                packet.push_back(18);
                codeLengthsAlphabetCodeLengths.push_back(18);
                packetLength = min((unsigned int)138, length);
                length -= packetLength;
                packetLength -= 11;

            } else if(length <= 10) {

                packet.push_back(17);
                codeLengthsAlphabetCodeLengths.push_back(17);

                packetLength = min((unsigned int)10, length);
                length -= packetLength;
                packetLength -= 3;

            }

            packet.push_back(packetLength);

        } else {

            for(unsigned int i = 0; i < length; ++i){
                packet.push_back(0);
                codeLengthsAlphabetCodeLengths.push_back(0);
            }
            length = 0;
        }
    }

    return packet;
}

CodeLengths makeCodeLengths(
    vector<unsigned int> fileData,
    unsigned int alphabetSize,
    CodeLength maxCodeLength)
{
    /* Handle the special case in which there is no data at all. */
    if(fileData.size() == 0 || fileData.size() == 1){
        CodeLengths cls(alphabetSize);
	fill(cls.begin(), cls.end(), 0);

	if(fileData.size() == 0)
	    return cls;
	else {
	    cls[fileData.front()] = 1;
	    return cls;
	}
    }
    FrequencyTable freqTable = constructFrequencyTable(fileData, alphabetSize);

    return makeCodeLengths(freqTable, maxCodeLength);
}

void permuteCodelengths(CodeLengths & cs)
{
    CodeLengths permuted(19);
    unsigned int perm[] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    for(unsigned int i = 0; i < 19; ++i)
        permuted[i] = cs[perm[i]];

    cs = permuted;
}

RevCodesList loadUsingCodeLengthCodes(
    unsigned short length,
    unsigned short alphabetLength,
    const RevCodesList & codeLengthCodes,
    BitReader * compressedStream)
{
    size_t i;

    printf("alphabetLength:%d\n", alphabetLength);
    printf("length:%d\n", length);

    CodeLengths codeLengths(alphabetLength);
    unsigned int translatedCode;
    int codesLenI;

    codesLenI = 0;

    fill(codeLengths.begin(), codeLengths.end(), 0);

    while(1){
        /* The codes are only applied to the codes in the alphabet, NOT the repetition factors */
        translatedCode = readCode(codeLengthCodes,compressedStream);

        if(/*code.litteralValue >= 0 &&*/ translatedCode <= 15){

            codeLengths[codesLenI++] = translatedCode;

        } else if(translatedCode >= 16){

            vector<BYTE> repeated;

            /* TODO: does this work????*/
            if(translatedCode == 16){
                repeated =
                    repeatPreviousLengthCode(
                        codeLengths[codesLenI-1],
                        compressedStream);
            }else if(translatedCode == 17){
                repeated = repeatZeroLengthCode(3,3, compressedStream);
            } else if(translatedCode == 18){

                repeated = repeatZeroLengthCode(11,7, compressedStream);
            }

            for(i = 0; i < repeated.size(); ++i){
                codeLengths[codesLenI++] = repeated[i];
            }
        }

        if(codesLenI == length){
            break;
        }
    }

    printf("codeLengths\n");
    printCodeLengths(codeLengths);


    return reverseCodesList(translateCodes(codeLengths));
}

void printCodeLengths(CodeLengths cs){
    copy(
        cs.begin(),
        cs.end(),
        ostream_iterator<CodeLength>(cout, ", "));

    cout << "\n";
}

void writeCompressedCodeLengths(
    CodeLengths compressedCodeLengths,
    CodesList codeLengthCodes,
    BitWriter * outBits)
{
    for(size_t i = 0; i < compressedCodeLengths.size(); ++i){
        HuffmanCode code;
        if(compressedCodeLengths[i] <= 15){
            code = codeLengthCodes[compressedCodeLengths[i]];

            writeCode(code, outBits);
        } else if (compressedCodeLengths[i] == 16){
            code = codeLengthCodes[compressedCodeLengths[i]];
            writeCode(code, outBits);

            /* Write the repeat code. */
            ++i;
            outBits->writeBits(compressedCodeLengths[i], 2);
        } else if (compressedCodeLengths[i] == 17){
            code = codeLengthCodes[compressedCodeLengths[i]];
            writeCode(code, outBits);

            /* Write the repeat code. */
            ++i;
            outBits->writeBits(compressedCodeLengths[i], 3);
        } else if (compressedCodeLengths[i] == 18){
            code = codeLengthCodes[compressedCodeLengths[i]];
            writeCode(code, outBits);

            /* Write the repeat code. */

            ++i;

            outBits->writeBits(compressedCodeLengths[i], 7);
        }
    }
}

size_t findBegTrail(CodeLengths codeLengthCodeLengths)
{
    size_t begTrail;
    for(begTrail = codeLengthCodeLengths.size() - 1; begTrail != 0; --begTrail)
        if(codeLengthCodeLengths[begTrail] != 0)
            return begTrail;

    return begTrail;
}

/* First writes HCLEN and then writes the code lengths for the code length alphabet. */
void writeCodeLengthCodeLengths(
    CodeLengths codeLengthCodeLengths,
    BitWriter * outBits)
{
    permuteCodelengths(codeLengthCodeLengths);

    /* Find the end of the trailing zeroes. */
    size_t begTrail = findBegTrail(codeLengthCodeLengths);
    outBits->writeBits(begTrail + 1 - 4, 4);

    /* these codes are used to encode the byte codes. */
    for(size_t i = 0; i < (begTrail + 1); ++i){
        outBits->writeBits(codeLengthCodeLengths[i], 3);
    }
}

CodeLengths cutTrailingZeroCodeLengths(CodeLengths codeLengths, size_t minSize)
{
    size_t begTrail = findBegTrail(codeLengths);
    size_t size = begTrail + 1;

    if(size < minSize)
	size = minSize;

    codeLengths.resize(size);

    return codeLengths;
}
