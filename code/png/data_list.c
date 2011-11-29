#include "data_list.h"
#include <stdio.h>

DataList  allocateNewDataList(size_t size, size_t maxCount, size_t count)
{
    DataList list;

    list.list = malloc(size * maxCount);
    list.size = size;

    list.count = count;
    list.maxCount = maxCount;

    return list;
}

void freeDataList(DataList list)
{
    if(list.list != NULL /*|| list.count != 0*/)
	free(list.list);

    list.list = NULL;
    list.size = 0;

    list.maxCount = 0;
    list.count = 0;
}

void printDataList(DataList list, void (*printFunc)(void *))
{
    size_t i;

    for(i = 0; i < list.count; ++i)
	printFunc(list.list[i]);
}

DataList getNewDataList(size_t size)
{
    DataList list;

    list.list = NULL;
    list.size = size;

    list.maxCount = 0;
    list.count = 0;

    return list;
}

DataList accommodateDataListCount(DataList oldList, size_t newCount)
{
    size_t i, transfer;
    DataList newList;

    newList = allocateNewDataList(oldList.size, newCount, oldList.count);

    /* If all of the data in the old container fit into the new. */
    if(oldList.count <= newCount)
	transfer = oldList.count;
    else
	/* If not all the data in the old container fit in the new. */
	transfer = newCount;

    for(i = 0; i < transfer; ++i)
	newList.list[i] = oldList.list[i];

    /* Free any memory in the previous container. */
    freeDataList(oldList);

    return newList;
}

DataList growDataListCount(DataList oldList)
{
    size_t newCount;

    if(oldList.count == oldList.maxCount){

	newCount = oldList.count;

	if(newCount == 0)
	    newCount = 1;
	else
	    newCount *= DATA_LIST_GROWTH_FACTOR;

	return accommodateDataListCount(oldList, newCount);
    } else
	return oldList;
}

void addToDataList(DataList * list, void * newElement)
{
    *list = growDataListCount(*list);

    list->list[list->count++] = newElement;
}
