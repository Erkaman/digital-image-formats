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
    std::vector<unsigned int> data,
    unsigned int alphabetSize,
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

CodeLengths compressCodeLengths(
    const CodeLengths & codeLengths,
    CodeLengths & codeLengthsAlphabetCodeLengths);


template<typename InputIterator, typename T>
InputIterator
find_first_not_of(InputIterator first, InputIterator last, const T & val)
{
    for (first = first;
         first != last;
         ++first)
        if (*first != val)
            return first;

    return last;;
}


CodeLengths makePacket(
    CodeLength length,
    unsigned int code,
    CodeLengths & codeLengthsAlphabetCodeLengths);

CodeLengths makeRepeatPacket(
    CodeLength length,
    unsigned int code,
    CodeLengths & codeLengthsAlphabetCodeLengths);
CodeLengths makeZeroPacket(
    CodeLength length,
    CodeLengths & codeLengthsAlphabetCodeLengths);

void permuteCodelengths(CodeLengths & cs);

RevCodesList loadUsingCodeLengthCodes(
    unsigned short length,
    unsigned short alphabetLength,
    const RevCodesList & codeLengthCodes,
    BitReader * compressedStream);

void printCodeLengths(CodeLengths cs);

CodeLengths extractAlphabetCodeLengths(CodeLengths cs);

void writeCodeLengthCodeLengths(
    CodeLengths codeLengthCodeLengths,
    BitWriter * outBits);

void writeCompressedCodeLengths(
    CodeLengths compressedCodeLengths,
    CodesList codeLengthCodes,
    BitWriter * outBits);

#endif /* _HUFFMAN_H_ */

