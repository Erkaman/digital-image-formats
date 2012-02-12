#include "stlutil.h"

using std::vector;

vector<unsigned int> readFile(FILE * in)
{
    vector<unsigned int> data;

    while(true){
	int b = getc(in);
	if(b == EOF)
	    break;

	data.push_back(b);
    }

    return data;
}

std::vector<BYTE> readFileBytes(FILE * in)
{
    vector<unsigned int> data = readFile(in);
    vector<BYTE> bs(data.size());

    for(size_t i = 0; i < data.size(); ++i)
	bs[i] = (BYTE)data[i];

    return bs;
}
