#include "data_list.h"
#include <stdio.h>

DataList allocateNewDataList(
    size_t maxCount,
    size_t count,
    void (*freeFunction) (void *),
    void * (*copyFunction) (void *))
{
    DataList list;

    list.list = malloc(ELEMENT_SIZE * maxCount);

    list.count = count;
    list.maxCount = maxCount;
    list.freeFunction = freeFunction;
    list.copyFunction = copyFunction;

    return list;
}

void freeDataList(DataList list, int freeElements)
{
    size_t i;

    if(freeElements)
        for(i = 0; i < list.count; ++i)
            if(list.freeFunction == NULL)
                free(list.list[i]);
            else
                list.freeFunction(list.list[i]);

    if(list.list != NULL /*|| list.count != 0*/)
        free(list.list);

    list.list = NULL;
    list.copyFunction = NULL;
    list.freeFunction = NULL;

    list.maxCount = 0;
    list.count = 0;
}

void printDataList(DataList list, void (*printFunc)(void *))
{
    size_t i;

    for(i = 0; i < list.count; ++i)
        printFunc(list.list[i]);
}

DataList getNewDataList(
    void (*freeFunction) (void *),
    void * (*copyFunction) (void *))
{
    DataList list;

    list.list = NULL;
    list.freeFunction = freeFunction;
    list.copyFunction = copyFunction;

    list.maxCount = 0;
    list.count = 0;

    return list;
}

DataList accommodateDataListCount(DataList oldList, size_t newCount)
{
    size_t i, transfer;
    DataList newList;

    newList = allocateNewDataList(
        newCount,
        oldList.count,
        oldList.freeFunction,
        oldList.copyFunction);

    /* If all of the data in the old container fit into the new. */
    if(oldList.count <= newCount)
        transfer = oldList.count;
    else
        /* If not all the data in the old container fit in the new. */
        transfer = newCount;

    for(i = 0; i < transfer; ++i)
        newList.list[i] = oldList.list[i];

    /* Free any memory in the previous container, except for the elements. */
    freeDataList(oldList,0);

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

void copyAppendToDataList(DataList * destination, DataList appendee)
{
    size_t i;

    for(i = 0; i < appendee.count; ++i){
        addToDataList(destination,destination->copyFunction(appendee.list[i]));
    }
}

DataList readBytes(size_t count, FILE * in)
{
    DataList list;
    size_t i;

    list = getNewDataList(NULL, copyByte);

    for(i = 0; i < count; ++i)
        addByteToDataList(&list, (BYTE)getc(in));

    return list;
}

void addByteToDataList(DataList * list, BYTE b)
{
    BYTE * bp;

    bp = malloc(sizeof(BYTE));
    *bp = b;
    addToDataList(list, bp);
}

void * copyByte(void * vptr)
{
    BYTE * copy;
    BYTE b;

    b = *(BYTE *)vptr;

    copy = malloc(sizeof(BYTE));
    *copy = b;

    return copy;
}

void writeData(DataList data, FILE * out)
{
    size_t i;

    for(i = 0; i < data.count; ++i)
        putc(*(BYTE *)data.list[i],out);
}

