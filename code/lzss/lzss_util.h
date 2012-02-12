#ifndef _LZSS_UTIL_H_
#define _LZSS_UTIL_H_

#include "../defs.h"

#include <vector>

enum TokenType{
    OffsetLengthToken,
    SymbolToken
};

class Token{

public:

    TokenType type;

    unsigned int offset;
    unsigned int length;
    BYTE symbol;
};

std::vector<Token> compress(
    unsigned int windowSize,
    unsigned int lookAheadSize,
    const std::vector<BYTE> & data);

void decodeToken(Token token, std::vector<BYTE> & decompressed);

#endif /* _LZSS_UTIL_H_ */
