#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "../defs.h"


class Token{

public:

    signed long offset;
    signed long length;
    BYTE symbol;
};

const int symbolSize = 8;

#endif /* _TOKEN_H_ */
