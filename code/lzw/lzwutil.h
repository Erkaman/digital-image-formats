#ifndef _LZWUTIL_H_
#define _LZWUTIL_H_

#include <map>
#include <vector>
#include <cstdio>

typedef signed long code;
typedef std::pair<code, code> codeStr;

const code emptyCh = -1;

code outputCodes(
    codeStr cs,
    const std::vector<codeStr> & stringTable,
    std::map<codeStr, std::vector<code> > & cache,
    std::FILE * out);

std::vector<code> outputCodes(
    codeStr cs,
    const std::vector<codeStr> & stringTable,
    std::map<codeStr, std::vector<code> > & cache);

std::vector<code> codeStrToVector(codeStr cs,const std::vector<codeStr> & stringTable);

#endif /* _LZWUTIL_H_ */
