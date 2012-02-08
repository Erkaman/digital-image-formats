#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#include "../frequency_table.h"
#include "../bits.h"
#include "tree.h"
#include <vector>

struct HuffmanCode{
    /* the code value of the code*/
    unsigned long value;

    /* The codes whose lengths are zero are ignored.  */
    int codeLength;
};

typedef struct{
    BYTE symbol;
    unsigned long frequency;
} AlphabetSymbol;


typedef std::vector<HuffmanCode> CodesList;

Node * constructHuffmanTree(FrequencyTable freqTable);

CodesList createOptimumCodes(Node * huffmanTree);

void printCodesList(CodesList codes);

CodesList translateCodes(const std::vector<unsigned long> & codeLengths);

void writeCode(HuffmanCode code, BitWriter * outBits);



bool huffmanCodeCompare(const HuffmanCode & a, const HuffmanCode & b);

struct HuffmanCodeCompare {
    bool operator() (const HuffmanCode & a, const HuffmanCode & b)const
        {
            return (a.value < b.value) || (a.codeLength < b.codeLength);
        }

};

unsigned long readCode(
    const std::map<HuffmanCode, unsigned long, HuffmanCodeCompare> & codes,
    BitReader * inBits);


std::map<HuffmanCode, unsigned long, HuffmanCodeCompare>
reverseCodesList(const CodesList & codes);


#endif /* _HUFFMAN_H_ */

