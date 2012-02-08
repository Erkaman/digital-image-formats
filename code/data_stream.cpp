#include "data_stream.h"
#include <cmath>
#include "io.h"
#include <algorithm>

using std::vector;
using std::distance;

DataStream::DataStream(vector<BYTE> & list, Endian endian_)
{
    this->iter = list.begin();
    this->beg = list.begin();;
    this->streamSize = list.size();
    this->endian = endian_;
    this->inputType = vec;
}

size_t DataStream::getPosition()const
{
    return distance(this->beg, this->iter);
}


size_t DataStream::getStreamSize()const
{
    return this->streamSize;
}

BYTE DataStream::readStreamByte()
{
    BYTE b;
    b = *iter;
    ++iter;
    return b;
}

unsigned int DataStream::read16BitsNumber()
{
    BYTE b1,b2;
    unsigned int n;

    b1 = this->readStreamByte();
    b2 = this->readStreamByte();

    if(this->endian == BigEndian)
        n = b1 * 256 + b2;
    else if(this->endian == SmallEndian)
        n = b2 * 256 + b1;

/*    if(stream->endian == ENDIAN_BIG){
      n =  ntohs(n);

      } else if(stream->endian == ENDIAN_SMALL){

      n = htons(n);

      }*/

    return n;
}

unsigned long DataStream::read32BitsNumber()
{
    BYTE b1, b2, b3, b4;

    b1 = this->readStreamByte();
    b2 = this->readStreamByte();
    b3 = this->readStreamByte();
    b4 = this->readStreamByte();

    return b1 * pow(256,3) + b2 * pow(256, 2) + b3 * pow(256,1) + b4 * pow(256,0);
}
