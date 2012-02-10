#ifndef _HUFFMAN_H_
#define _HUFFMAN_H_

#include "../frequency_table.h"
#include "../bits.h"
#include <vector>
#include <map>

typedef unsigned int CodeLength;
typedef std::vector<CodeLength> CodeLengths;

struct HuffmanCode{
    /* the code value of the code*/
    unsigned long value;

    /* The codes whose lengths are zero are ignored.  */
    CodeLength length;
};

typedef std::vector<HuffmanCode> CodesList;

#define CODE_LENGTH_CODES 19

CodeLengths makeCodeLengths(
    FrequencyTable freqTable,
    CodeLength maxCodeLength);


void printCodesList(CodesList codes);

CodesList translateCodes(const CodeLengths & codeLengths);

void writeCode(HuffmanCode code, BitWriter * outBits);

bool huffmanCodeCompare(const HuffmanCode & a, const HuffmanCode & b);

struct HuffmanCodeCompare {
    bool operator() (const HuffmanCode & a, const HuffmanCode & b)const
        {
            return (a.value < b.value) || (a.length < b.length);
        }
};

typedef std::map<HuffmanCode, unsigned long, HuffmanCodeCompare> RevCodesList;


unsigned long readCode(
    const RevCodesList & codes,
    BitReader * inBits);

RevCodesList reverseCodesList(const CodesList & codes);

RevCodesList loadCodeLengthCodes(unsigned int HCLEN,BitReader * compressedStream);

std::vector<BYTE> repeatZeroLengthCode(
    unsigned int minCodeLength,
    unsigned int extraBits,
    BitReader * compressedStream);

std::vector<BYTE> repeatPreviousLengthCode(
    unsigned int previousCode,
    BitReader * compressedStream);

#endif /* _HUFFMAN_H_ */
