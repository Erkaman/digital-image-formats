#ifndef _FREQUENCY_TABLE_H_
#define _FREQUENCY_TABLE_H_

#include <vector>
#include <cstdio>
#include "defs.h"

/* Frequency table for integer types*/
class FrequencyTable {

private:

    std::vector<unsigned long> freqs;

    void constructFrequencyTable(
	const std::vector<unsigned long> & data);

public:

    FrequencyTable(const std::vector<BYTE> & data);
    FrequencyTable(const std::vector<unsigned long> & data);
    FrequencyTable(FILE * in);

    unsigned long operator [] (size_t index);

    void set(size_t index, unsigned long val);

    size_t size() const;
};

#endif /* _FREQUENCY_TABLE_H_ */
