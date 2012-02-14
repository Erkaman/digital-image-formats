#ifndef _BITS_H_
#define _BITS_H_

#include <cstdio>
#include <vector>
#include "defs.h"

int getbits(unsigned long b, int start, int end);

enum BitOrder {
    MSBF,
    LSBF
};

class BitReader {
private:
    BYTE byte;
    BYTE bits;

    BitOrder order;

    BYTE readBit(void);

protected:

    BitReader(BitOrder order_);
public:

    virtual BYTE nextByte() = 0;

    virtual size_t getPosition()const = 0;

    unsigned int readBits(unsigned int nbits);

};

class BitFileReader : public BitReader{

private:

    FILE * in;
    fpos_t beg;

public:
    BitFileReader(FILE * in_, BitOrder order_);

    size_t getPosition()const;
    BYTE nextByte();
};

class BitIterReader : public BitReader{

private:

    typedef std::vector<BYTE>::iterator Iter;

    Iter iter, beg;

public:
    BitIterReader(Iter iter_, BitOrder order_);

    BYTE nextByte();
    size_t getPosition()const;
};

class BitWriter {
private:

    BYTE byte;
    BYTE bits;

    BitOrder order;


    void writeBit(BYTE bit);

protected:

    virtual void writeByte(BYTE b) = 0;
    BitWriter(BitOrder order_);


public:

    void writeBits(unsigned int data, unsigned int nbits);
};

class BitFileWriter : public BitWriter {

private:

    FILE * out;

public:

    BitFileWriter(FILE * out_, BitOrder order);

    void writeByte(BYTE b);
};

class BitVectorWriter : public BitWriter {

private:

    std::vector<BYTE> output;

public:

    BitVectorWriter(BitOrder order);

    void writeByte(BYTE b);

    std::vector<BYTE> getOutputVector();
};



#endif /* _BITS_H_ */
