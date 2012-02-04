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
    const vector<codeStr> & stringTable,
    map<codeStr, vector<code> > & cache)
{
    vector<code> outStr;

    map<codeStr, vector<code> >::iterator iter;

    iter = cache.find(cs);

    /* in the cache */
    if(iter != cache.end())
	outStr = iter->second;
    else{
	outStr = codeStrToVector(cs, stringTable);
	cache[cs] = outStr;
    }

    return outStr;
}

code outputCodes(
    codeStr cs,
    const vector<codeStr> & stringTable,
    map<codeStr, vector<code> > & cache,
    FILE * out)
{
    vector<code> outStr;

    map<codeStr, vector<code> >::iterator iter;

    iter = cache.find(cs);

    /* in the cache */
    if(iter != cache.end())
	outStr = iter->second;
    else{
	outStr = codeStrToVector(cs, stringTable);
	cache[cs] = outStr;
    }

    for(size_t i = 0;
        i < outStr.size();
        ++i)
        putc(outStr[i],out);

    return outStr[0];
}
