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
