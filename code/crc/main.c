#include <cstdio>
#include "../defs.h"
#include "../data_list.h"

unsigned long crc(DataList data, unsigned long poly);
unsigned long crc8(DataList list, unsigned long polynom, unsigned long initial);
int main(void)
{
    DataList data;

    data = getNewDataList(NULL, NULL);

    addByteToDataList(&data, 0x99);

    printf("Final CRC: %lx\n", crc8(data, 0x107, 0));

    freeDataList(data, 1);

    return 0;
}

/* Take single byte, from paint less explanation, then calculate the checksum. */
unsigned long crc(DataList data, unsigned long poly)
{
    unsigned long reminder = 0/*xFFFFFFFF*/; /* standard initial value in CRC32 */
    unsigned long i;
    unsigned long bit;
    BYTE b;

    for(i = 0; i < data.count; ++i){
        b = *(BYTE *)data.list[i];
        printf("b:%x\n", b);
        reminder = reminder ^ b; /* must be zero extended */

        printf("reminder after first xor:%lx\n", reminder);

        for(bit = 0; bit < 8; bit++){

            printf("bit n:%ld\n", bit);

            if(reminder & 0x01){

                printf("If branch\n");

                reminder = (reminder >> 1) ^ poly;

                printf("reminder:%lx\n", reminder);
            }
            else{
                printf("Else branch\n");

                reminder >>= 1;

                printf("reminder:%lx\n", reminder);

            }
        }
    }

    return reminder/* ^ 0xFFFFFFFF*/;
}

/* polynom = 0x07*/
/* intiial = 0?*/

unsigned long crc8(DataList list, unsigned long polynom, unsigned long initial)
{
    unsigned long i,j, result;
    BYTE b;

    result = 0;

    result = initial;
    for(i = 0; i < list.count; ++i){

        b = *(BYTE *)list.list[i];
        result = result ^ b;

	printf("after xor: %lx\n", result);

        for( j=0; j < 8; ++j){

	    /* This to do the actual division. */
            if (result & 0x80){
		printf("if branch\n");
                result = (result << 1) ^ polynom;
	    }
            else{
		printf("else branch\n");
                result = result << 1;
	    }

	    printf("result: %lx\n", result);
        }
    }

    return result & 0xff;
}

