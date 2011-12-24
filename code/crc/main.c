#include <stdio.h>
#include "../defs.h"
#include "../data_list.h"

unsigned long crc(DataList data, unsigned long poly);

int main(void)
{
    DataList data;

    data = getNewDataList(NULL, NULL);

    addByteToDataList(&data, 0x57);

    printf("%lx\n", crc(data ,0x107));

    freeDataList(data, 1);

    return 0;
}

/* Take single byte, from paint less explanation, then calculate the checksum. */
unsigned long crc(DataList data, unsigned long poly)
{
    unsigned long reminder = 0; /* standard initial value in CRC32 */
    unsigned long i;
    unsigned long bit;
    BYTE b;

    for(i = 0; i < data.count; ++i){
        b = *(BYTE *)data.list[i];
        reminder ^= b; /* must be zero extended */
        for(bit = 0; bit < 8; bit++)
            if(reminder & 0x01)
                reminder = (reminder >> 1) ^ poly;
            else
                reminder >>= 1;
    }

    return reminder;
}
