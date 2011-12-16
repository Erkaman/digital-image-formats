#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include "../common.h"

void testDataList(void);

void print_size_t(void * s);

int main(void)
{
    testDataList();

    return 1;
}

void print_size_t(void * s)
{
    printf("%d\n",*(short *)s);
}

void testDataList(void)
{
    DataList list;
    short i;
    short * p;

    list = getNewDataList(NULL);

    for(i = 0; i < 1000; ++i){
	p = malloc(sizeof(short));
	*p = i;

       addToDataList(&list, p);
    }

    printDataList(list,print_size_t);

    freeDataList(list, 1);
}
