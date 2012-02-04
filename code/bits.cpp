#include "bits.h"
#include <algorithm>

using std::distance;

BitReader::BitReader(FILE * in_, BitOrder order)
{
    this->in = in_;
    this->inDataType = FP;

    this->byte = 0;
    this->bits = 0;
    this->bitOrder = order;
}

BitReader::BitReader(BYTE * data_, BitOrder order)
{
    this->data = data_;
    this->inDataType = BP;

    this->byte = 0;
    this->bits = 0;
    this->bitOrder = order;
}

BitReader::BitReader(std::vector<BYTE>::const_iterator data_, BitOrder order)
{
    this->iter = data_;
    this->beg = this->iter;
    this->inDataType = ITER;

    this->byte = 0;
    this->bits = 0;
    this->bitOrder = order;
}

size_t BitReader::getPosition()const
{
    return distance(this->beg, this->iter);
}

void BitReader::nextByte(void)
{
    if(inDataType == FP)
        fread(
            &this->byte,
            sizeof(BYTE),
            1,
            this->in);
    else if(inDataType == BP){
        this->byte = *this->data;
        ++this->data;
    }
    else if(inDataType == ITER){
        this->byte = *this->iter;
        ++this->iter;
    }
    this->bits = 8;
}

BYTE BitReader::readBit(void)
{
    BYTE ret = 0;

    if (this->bits == 0) {
	nextByte();
    }

    if(this->bitOrder == MSBF){
        ret = (this->byte & 0x80) >> 7;
        this->byte <<= 1;
    } else if(this->bitOrder == LSBF){
        ret = this->byte & 0x01;
        this->byte >>= 1;
    }

    this->bits--;
    return ret & 1;
}

unsigned int BitReader::readBits(unsigned int nbits)
{
    unsigned int ret = 0;
    unsigned int orig = nbits;

    BYTE b;

    for (; nbits; --nbits){

        b = this->readBit();

        if(this->bitOrder == LSBF)
            ret |= b << (orig - nbits);
        else if(this->bitOrder == MSBF)
            ret |= b << (nbits - 1);
    }

    return ret;
}

BitWriter::BitWriter(FILE * out_, BitOrder order)
{
    this->out = out_;

    this->byte = 0;
    this->bits = 0;

    this->bitOrder = order;
}

void BitWriter::writeBit(BYTE bit)
{
    if(this->bitOrder == MSBF){
        this->byte |= (bit & 1) << (7 - this->bits);
    } else if(this->bitOrder == LSBF){
        this->byte |= (bit & 1) << (this->bits);
    }

    this->bits++;

    if (this->bits == 8) {
        fwrite(&this->byte,sizeof(BYTE), 1, this->out);
        this->bits = 0;
        this->byte = 0;
    }
}

void BitWriter::writeBits(unsigned int data, unsigned int nbits)
{
    unsigned int b;

    for (unsigned i = 0; i < nbits; ++i){
        if(this->bitOrder == MSBF){
            b = (data & (1 << (nbits - i - 1))) >> (nbits - i - 1);
        } else if(this->bitOrder == LSBF){
            b = data & 0x01;
            data >>= 1;
        }

        this->writeBit(b);
    }
}
