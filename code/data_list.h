#ifndef _DATA_LIST_H_
#define _DATA_LIST_H_

#include <stdlib.h>

typedef struct {

    void ** list;

    /* the number of elements in the list. */
    size_t count;

    /* the maximum number of elements in the list. */
    size_t maxCount;

    void (*freeFunction) (void *);
    void * (*copyFunction) (void *);

} DataList;

#define DATA_LIST_GROWTH_FACTOR 2
#define ELEMENT_SIZE sizeof(void *)

DataList allocateNewDataList(
    size_t count,
    size_t position,
    void (*freeFunction) (void *),
    void * (*copyFunction) (void *));

void freeDataList(DataList list, int freeElements);
void printDataList(DataList list, void (*printFunc)(void *));

DataList getNewDataList(
    void (*freeFunction) (void *),
    void * (*copyFunction) (void *));

void print_size_t(void * s);
DataList accommodateDataListCount(DataList oldList, size_t newCount);
DataList growDataListCount(DataList oldList);
void addToDataList(DataList * list, void * newElement);

void copyAppendToDataList(DataList * destination, DataList appendee);

#endif /* _DATA_LIST_H_ */
