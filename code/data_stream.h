#ifndef _DATA_STREAM_H_
#define _DATA_STREAM_H_

#include "bitwise.h"
#include "defs.h"
#include <vector>
#include <cstdio>

enum Endian {
    BigEndian,
    SmallEndian
};

class DataStream{

private:

    enum InputType{
	vec,
	file
    };

    std::vector<BYTE>::const_iterator iter, beg;

    FILE * in;

    Endian endian;

    InputType inputType;

    size_t streamSize;

public:

    size_t getStreamSize()const;
    size_t getPosition()const;

    DataStream(std::vector<BYTE> & list, Endian endian_);

    BYTE readStreamByte();
    uint16_t read16BitsNumber();
    uint32_t read32BitsNumber();
};


#endif /* _DATA_STREAM_H_ */


