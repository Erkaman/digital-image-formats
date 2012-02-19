#include "lzwutil.h"
#include <algorithm>

using std::pair;
using std::vector;
using std::map;
using std::find;

vector<code> codeStrToVector(codeStr cs,const vector<codeStr> & stringTable)
{
    vector<code> outStr;

    while(cs.second != emptyCh){
        outStr.push_back(cs.second);
        cs = stringTable[cs.first];
    }
    outStr.push_back(cs.first);

    std::reverse(outStr.begin(), outStr.end());

    return outStr;
}

vector<code> outputCodes(
    codeStr cs,
    const vector<codeStr> & stringTable)
{
    vector<code> outStr;

    map<codeStr, vector<code> >::iterator iter;


    return codeStrToVector(cs, stringTable);
}

code outputCodes(
    codeStr cs,
    const vector<codeStr> & stringTable,
    FILE * out)
{
    vector<code> outStr;

    map<codeStr, vector<code> >::iterator iter;

    outStr = codeStrToVector(cs, stringTable);

    for(size_t i = 0;
        i < outStr.size();
        ++i)
        putc(outStr[i],out);

    return outStr[0];
}
