#include "stlutil.h"

using std::vector;

vector<BYTE> readFile(FILE * in)
{
    vector<BYTE> data;

    while(true){
	int b = getc(in);
	if(b == EOF)
	    break;

	data.push_back(b);
    }

    return data;
}
