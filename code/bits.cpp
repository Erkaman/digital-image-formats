#include "bits.h"
#include <algorithm>

using std::distance;

BitReader::BitReader(BitOrder order_)
{
    this->byte = 0;
    this->bits = 0;
    this->order = order_;
}

BYTE BitReader::readBit(void)
{
    BYTE ret = 0;

    if (this->bits == 0) {
        this->byte = nextByte();
        this->bits = 8;
    }

    if(this->order == MSBF){
        ret = (this->byte & 0x80) >> 7;
        this->byte <<= 1;
    } else if(this->order == LSBF){
        ret = this->byte & 0x01;
        this->byte >>= 1;
    }

    this->bits--;
    return ret & 1;
}

BYTE BitFileReader::nextByte()
{
    return getc(in);
}

unsigned int BitReader::readBits(unsigned int nbits)
{
    unsigned int ret = 0;
    unsigned int orig = nbits;

    BYTE b;

    for (; nbits; --nbits){

        b = this->readBit();

        if(this->order == LSBF)
            ret |= b << (orig - nbits);
        else if(this->order == MSBF)
            ret |= b << (nbits - 1);
    }

    return ret;
}

BitFileReader::BitFileReader(FILE * in_, BitOrder order_):
    BitReader(order_)
{
    this->in = in_;
    fgetpos(this->in, &this->beg);
}

size_t BitFileReader::getPosition()const
{
    fpos_t cur;
    size_t s2,s1;
    fgetpos(this->in, &cur);

    s2 = ftell(in);

    fsetpos(this->in,&this->beg);
    s1 = ftell(in);

    fsetpos(in,&cur);

    return s2 - s1;
}

BitIterReader::BitIterReader(Iter iter_, BitOrder order_):
    BitReader(order_)
{
    this->iter = iter_;
    beg = this->iter;
}

BYTE BitIterReader::nextByte()
{
    BYTE b = *iter;
    ++iter;
    return b;
}

size_t BitIterReader::getPosition()const
{
    return distance(this->beg, this->iter);
}


BitWriter::BitWriter(BitOrder order_)
{
    this->byte = 0;
    this->bits = 0;
    this->order = order_;
}

void BitWriter::writeBit(BYTE bit)
{
    if(this->order == MSBF){
        this->byte |= (bit & 1) << (7 - this->bits);
    } else if(this->order == LSBF){
        this->byte |= (bit & 1) << (this->bits);
    }

    this->bits++;

    if (this->bits == 8) {
        writeByte(this->byte);
        this->bits = 0;
        this->byte = 0;
    }
}

void BitWriter::writeBits(unsigned int data, unsigned int nbits)
{
    unsigned int b;

    for (unsigned i = 0; i < nbits; ++i){
        if(this->order == MSBF){
            b = (data & (1 << (nbits - i - 1))) >> (nbits - i - 1);
        } else if(this->order == LSBF){
            b = data & 0x01;
            data >>= 1;
        }

        this->writeBit(b);
    }
}

BitFileWriter::BitFileWriter(FILE * out_, BitOrder order_):BitWriter(order_)
{
    this->out = out_;
}

void BitFileWriter::writeByte(BYTE b)
{
    fwrite(&b,sizeof(BYTE), 1, this->out);
}

int getbits(int b, int start, int end)
{
    int len = end - start + 1;
    return (b >> start) & ~(~0 << (len));
}
