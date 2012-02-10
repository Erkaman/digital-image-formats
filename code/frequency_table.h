#ifndef _FREQUENCY_TABLE_H_
#define _FREQUENCY_TABLE_H_

#include <vector>
#include <cstdio>
#include "defs.h"

/* Frequency table for integer types*/
typedef std::vector<unsigned long> FrequencyTable;

FrequencyTable constructFrequencyTable(const std::vector<BYTE> & data, size_t size);
FrequencyTable constructFrequencyTable(const std::vector<unsigned int> & data, size_t size);
FrequencyTable constructFrequencyTable(FILE * in, size_t size);

#endif /* _FREQUENCY_TABLE_H_ */

