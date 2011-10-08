#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common.h"
#include "util.h"

void printHelp(void);

/* compression */

void lzw_compress(FILE * in,FILE * out);

void outputCode(unsigned int code,FILE * out);

int findMatch(unsigned int hash_prefix,unsigned int hash_character);

/* decompression */

void lzw_decompress(FILE * in,FILE * out);


void translateCode(unsigned int newCode);
char printString(FILE * out);

unsigned int inputCode(FILE *input);

#define BITS 15

#define HASHING_SHIFT (BITS-8)

#if BITS == 15
#define SIZE 33941
#endif
#if BITS == 14
#define SIZE 18041
#endif
#if BITS == 13
#define SIZE 9029
#endif
#if BITS <= 12
#define SIZE 5021
#endif

#define MAX_VALUE (1 << BITS) - 1
#define MAX_CODE MAX_VALUE - 1

typedef struct {
    unsigned int stringCode;
    unsigned int characterCode;
} tableEntry;

tableEntry * stringTable;
int * codeValues;

char stringCodeStack[40000];
int stackp;

unsigned int dictionaryIndex;

int main(int argc, char *argv[])
{
    int decompress = 0;
    FILE * in;
    FILE * out;
    char * outFile;
    char * inFile;
    char extension[5];

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else{

        ++argv;

        if(!strcmp("--help",*argv)){
            printHelp();
            return 0;
        }
        if(!strcmp("-d",*argv)){
            decompress = 1;
            ++argv;
        }

        stringTable = (tableEntry *)malloc(sizeof(tableEntry) * SIZE);
        codeValues = (int *)malloc(sizeof(int) * SIZE);

        inFile = *argv;
        in = fopen(inFile,"rb");
        assertFileOpened(in);

        if(decompress){
            strncpyBack(extension,*argv,4);

            if(!strcmp(extension,".lzw")){
                outFile = changeExtension(*argv,"unc");

                out = fopen(outFile,"wb");
                free(outFile);
                assertFileOpened(out);
            }else{

                outFile = strAppend(*argv,".unc");
                out = fopen(outFile,"wb");
                free(outFile);
                assertFileOpened(out);
            }

            lzw_decompress(in,out);

/*            printf("String table\n");
              for(i = 0;i < 11; ++i){
              printf("%d : %d,%d\n",
              i+256,stringTable[i+256].stringCode,
              stringTable[i+256].characterCode);
              }*/

        }else{
            outFile = strAppend(*argv,".lzw");
            out = fopen(outFile,"wb");
            free(outFile);
            assertFileOpened(out);

            lzw_compress(in,out);
        }

        fclose(in);
        fclose(out);

    }

    return 0;
}

void printHelp(void)
{
    printf("Usage: lzw IN\n");
    printf("Compress or decompress a LZW encoded file.\n"
           "Compression is done by default");
    printf("  --help\tDisplay this help message.\n");
    printf("  -d\tPerform decompression.\n");

}

void translateCode(unsigned int newCode)
{
    tableEntry entry;

/*    printf("translateCode\n");

      printf("newCode:%d\n",newCode); */

    entry = stringTable[newCode];

    while(1){

/*        printf("characterCode:%d\n",entry.characterCode); */
        stringCodeStack[stackp++] = entry.characterCode;

        if(entry.stringCode == ((unsigned int)-1) )
            break;
        else
            entry = stringTable[entry.stringCode];
    }
}

char printString(FILE * out)
{
    char returnValue;

    returnValue = stringCodeStack[stackp - 1];

    while(stackp > 0){
/*        printf("Stackoutput:%d\n",stringCodeStack[stackp-1]); */
        putc(stringCodeStack[--stackp],out);
    }

    return returnValue;
}

void lzw_decompress(FILE * in,FILE * out)
{
    unsigned int oldCode;
    unsigned int newCode;
    char character;

    for(dictionaryIndex = 0; dictionaryIndex < 256; ++dictionaryIndex){
        stringTable[dictionaryIndex].characterCode = dictionaryIndex;
        stringTable[dictionaryIndex].stringCode = (unsigned int) -1;
    }

    dictionaryIndex = 256;

    oldCode = inputCode(in);

    putc(oldCode,out);

    character = oldCode;

    newCode = inputCode(in);

    /* max_value should be checked for here but it doesn't work. */
    while (newCode != MAX_VALUE){

/*        printf("NEW LOOP\n");
          printf("newCode:%d\n",newCode);
          printf("oldCode:%d\n",oldCode);*/

        /* if it is not in the translation table. */
        if(!(newCode < dictionaryIndex)){
            /* wut*/
            translateCode(character);
            translateCode(oldCode);
        } else{
            translateCode(newCode);
        }

        character = printString(out);

	/* add it the table */
        if(dictionaryIndex <= MAX_CODE){

            stringTable[dictionaryIndex].stringCode = oldCode;
            stringTable[dictionaryIndex].characterCode = character;

	    dictionaryIndex++;
        }

        oldCode = newCode;
        newCode = inputCode(in);

/*        printf("newCode before end of loop:%d\n",newCode); */
    }

    out = out;
}

void lzw_compress(FILE * in,FILE * out)
{
    unsigned int charCode;
    unsigned int stringCode;
    /* rename the currentCode? */
    int nextCode;
    int index;


    for(nextCode = 0; nextCode < SIZE; ++nextCode)
        codeValues[nextCode] = -1;

    nextCode = 256;

    stringCode = getc(in);

    charCode = getc(in);

    while (charCode != (unsigned int)EOF){

        index = findMatch(stringCode,charCode);

        /* if it's in the table */
        if(codeValues[index] != -1){
            stringCode = codeValues[index];
        }else{
            /* not in the table */

            outputCode(stringCode,out);

            /* if less than the maximum size */
            if(nextCode <= MAX_CODE){
                stringTable[index].characterCode = charCode;
                stringTable[index].stringCode = stringCode;

                codeValues[index] = nextCode++;
            }

            stringCode = charCode;
        }

        charCode = getc(in);
    }

    outputCode(stringCode,out);
    outputCode(MAX_VALUE,out);
    outputCode(0,out);
}

void outputCode(unsigned int code,FILE * out)
{
    static int output_bit_count=0;
    static unsigned int output_bit_buffer=0L;

/*    printf("%d=%c\n",code,(char)(code)); */

    output_bit_buffer |= (unsigned int) code << (32 - BITS- output_bit_count);
    output_bit_count += BITS;
    while (output_bit_count >= 8)
    {
        putc(output_bit_buffer >> 24,out);
        output_bit_buffer <<= 8;
        output_bit_count -= 8;
    }
}

unsigned int inputCode(FILE *input)
{
    unsigned int return_value;
    static int input_bit_count=0;
    static unsigned int input_bit_buffer=0L;

    while (input_bit_count <= 24)
    {
        input_bit_buffer |=
            (unsigned int) getc(input) << (24-input_bit_count);
        input_bit_count += 8;
    }
    return_value=input_bit_buffer >> (32-BITS);
    input_bit_buffer <<= BITS;
    input_bit_count -= BITS;
    return(return_value);
}

int findMatch(unsigned int stringCode,unsigned int charCode)
{
    int index;
    int offset;

    index = (charCode << HASHING_SHIFT) ^ stringCode;
    if (index == 0)
        offset = 1;
    else
        offset = SIZE - index;
    while (1)
    {
        if (codeValues[index] == -1)
            return index;

        if (stringTable[index].stringCode == stringCode &&
            stringTable[index].characterCode == charCode)
            return index;

        index -= offset;

        if (index < 0)
            index += SIZE;
    }
}
