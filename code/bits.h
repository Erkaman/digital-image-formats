#ifndef _BITS_H_
#define _BITS_H_

#include <cstdio>
#include <vector>
#include "defs.h"

enum BitOrder {
    MSBF,
    LSBF
};

class BitReader {
private:
    BYTE byte;
    BYTE bits;

    BitOrder bitOrder;

    FILE * in;
    BYTE * data;
    std::vector<BYTE>::const_iterator iter, beg;


    enum InDataType{
	FP,
	BP,
	ITER
    };

    InDataType inDataType;


public:

    size_t getPosition()const;

    BitReader(FILE * in_, BitOrder order);
    BitReader(BYTE * data_, BitOrder order);
    BitReader(std::vector<BYTE>::const_iterator data_, BitOrder order);

    BYTE readBit(void);
    unsigned int readBits(unsigned int nbits);

    void writeBit(BYTE bit);

    void nextByte(void);
};

class BitWriter {
private:

    BYTE byte;
    BYTE bits;

    BitOrder bitOrder;

    FILE * out;

public:

    BitWriter(FILE * out_, BitOrder order);

    void writeBit(BYTE bit);

    void writeBits(unsigned int data, unsigned int nbits);

};


#endif /* _BITS_H_ */







