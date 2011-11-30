#include "fixed_data_list.h"

void freeFixedDataList(FixedDataList list, int freeElements)
{
    size_t i;

    if(freeElements){
	for(i = 0; i < list.count; ++i){
	    free(list.list[i]);
	}
    }

    if(list.list != NULL /*|| list.count != 0*/)
	free(list.list);

    list.list = NULL;
    list.count = 0;
}

void printFixedDataList(FixedDataList list, void (*printFunc)(void *))
{
    size_t i;

    for(i = 0; i < list.count; ++i)
	printFunc(list.list[i]);
}

FixedDataList getNewFixedDataList(size_t size, size_t count)
{
    FixedDataList list;

    list.list = malloc(size * count);
    list.count = count;

    return list;
}

