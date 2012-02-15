#include "frequency_table.h"
#include "stlutil.h"
#include <algorithm>

using std::vector;
using std::max_element;

FrequencyTable constructFrequencyTable(
    const std::vector<unsigned int> & data,
    size_t size)
{
    size_t i;

    FrequencyTable freqs;

    freqs.resize(size);

    fill(freqs.begin(),freqs.end(), 0);

    for(i = 0; i < data.size(); ++i)
        ++freqs[data[i]];

    return freqs;
}

FrequencyTable constructFrequencyTable(const std::vector<BYTE> & data, size_t size)
{
    vector<unsigned int> conv;

    for(size_t i = 0; i < data.size(); ++i)
        conv.push_back(data[i]);

    return constructFrequencyTable(conv, size);
}

FrequencyTable constructFrequencyTable(FILE * in, size_t size)
{
    vector<unsigned int> conv;
    vector<unsigned int> fileData;

    fileData = readFile(in);

    for(size_t i = 0; i < fileData.size(); ++i)
        conv.push_back(fileData[i]);

    return constructFrequencyTable(conv, size);
}
