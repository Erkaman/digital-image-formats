#include "frequency_table.h"
#include "stlutil.h"
#include <algorithm>

using std::vector;
using std::max_element;

void FrequencyTable::constructFrequencyTable(
    const std::vector<unsigned long> & data)
{
    size_t i;

    unsigned long max = *max_element(data.begin(), data.end());

    /* Also include the max.*/
    freqs.resize(max+1);

    for(i = 0; i < freqs.size(); ++i)
	freqs[i] = 0;

    for(i = 0; i < data.size(); ++i){
	unsigned long  v = data[i];
	++freqs[v];
    }
}

FrequencyTable::FrequencyTable(const vector<BYTE> & data)
{
    vector<unsigned long> conv;

    for(size_t i = 0; i < data.size(); ++i)
	conv.push_back(data[i]);

    constructFrequencyTable(conv);
}

FrequencyTable::FrequencyTable(const vector<unsigned long> & data)
{
    constructFrequencyTable(data);
}

FrequencyTable::FrequencyTable(FILE * in)
{
    vector<unsigned long> conv;
    vector<BYTE> fileData;

    fileData = readFile(in);

    for(size_t i = 0; i < fileData.size(); ++i)
	conv.push_back(fileData[i]);

    constructFrequencyTable(conv);
}

unsigned long FrequencyTable::operator [] (size_t index)
{
    return freqs[index];
}

size_t FrequencyTable::size() const
{
    return freqs.size();
}

void FrequencyTable::set(size_t index, unsigned long val)
{
    this->freqs[index] = val;
}
