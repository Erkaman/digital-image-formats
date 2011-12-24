#include <stdio.h>
#include "../defs.h"
#include "../data_list.h"

unsigned long crc(DataList data, unsigned long poly);

int main(void)
{
    DataList data;

    data = getNewDataList(NULL, NULL);

    addByteToDataList(&data, 0xd3);
    addByteToDataList(&data, 0xb0);

    printf("%lx\n", crc(data ,0x0b));

    freeDataList(data, 1);

    return 0;
}

unsigned long crc(DataList data, unsigned long poly)
{

    unsigned long reminder = 0xFFFF; /* standard initial value in CRC32 */
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

    return reminder ^ 0xFFFF;
}
