#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common.h"
#include "util.h"

/* TODO: go to line  91 NAO */
void printHelp(void);

/* compression */

void lzw_compress(FILE * in,FILE * out);

void outputCode(FILE * out,unsigned int code);
unsigned int inputCode(FILE *input);

void outputStringCode(FILE * out);

char * constructNewDictionaryEntry(unsigned int charCode);

void printStringCode(unsigned int stringCode);

void addToDictionary (int index,unsigned int charCode);

int find_code(unsigned int charCode);

/* decompression */

void lzw_decompress(FILE * in,FILE * out);

void addToTable(unsigned int oldCode,char character);


void translateCode(unsigned int newCode);
char printString(FILE * out);


#define BITS 12
#define SIZE 4096
#define MAX_VALUE (1 << BITS) - 1
#define MAX_CODE MAX_VALUE - 1

char ** dictionary;

typedef struct {
    unsigned int stringCode;
    unsigned int characterCode;
} tableEntry;

tableEntry * stringTable;

char stringCodeStack[40000];
int stackp;
int dictionarySize;
unsigned int dictionaryIndex;

int main(int argc, char *argv[])
{
    int decompress = 0;
    FILE * in;
    FILE * out;
    char * outFile;
    char * inFile;
    int i;
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

        dictionary = (char **)malloc(sizeof(char *) * SIZE);

        stringTable = (tableEntry *)malloc(sizeof(tableEntry) * SIZE);

        inFile = *argv;
        in = fopen(inFile,"rb");
        assertFileOpened(in);

        for(i = 0; i < SIZE; ++i)
            dictionary[i] = NULL;

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

            printf("String table\n");
            for(i = 0;i < 11; ++i){
                printf("%d : %d,%d\n",
                       i+256,stringTable[i+256].stringCode,
                       stringTable[i+256].characterCode);
            }

        }else{
            outFile = strAppend(*argv,".lzw");
            out = fopen(outFile,"wb");
            free(outFile);
            assertFileOpened(out);

            lzw_compress(in,out);


        }

        printf("Dictionary\n");
        for(i = 0;i < 11; ++i){
            printf("%d : %s\n",i + 256,dictionary[i]);
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

/*    printf("Grabbed entry\n"); */

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

void addToTable(unsigned int oldCode,char character)
{
    tableEntry newEntry;

/*    printf("addToTable code %d\n",dictionaryIndex);
    printf("oldCode:%d\n",oldCode);
    printf("character:%d\n",character); */

    newEntry.stringCode = oldCode;
    newEntry.characterCode = character;
    stringTable[dictionaryIndex++] = newEntry;
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

        if(dictionaryIndex <= MAX_CODE){
            addToTable(oldCode,character);
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
    int index;
    int formerIndex;

    dictionarySize = 0;


    formerIndex = -1;

    stringCodeStack[stackp++] = getc(in);

    charCode = getc(in);
    while (charCode != (unsigned int)EOF){

        index = find_code(charCode);

        if(dictionary[index] != NULL){
            stringCodeStack[stackp++] = charCode;
            formerIndex = index;
        }else{

            if(formerIndex != -1)
                outputCode(out,formerIndex + 256);
            else
                outputStringCode(out);

            formerIndex = -1;

            /* if less than the maximum size */
            if(dictionarySize <= MAX_CODE){
                dictionary[index] = constructNewDictionaryEntry(charCode);
                ++dictionarySize;
            }

            stackp = 0;

            stringCodeStack[stackp++] = charCode;
        }

        charCode = getc(in);
    }

    if(formerIndex != -1)
        outputCode(out,formerIndex + 256);
    else
        outputStringCode(out);

    outputCode(out,MAX_VALUE);
    outputCode(out,0);
}

void outputStringCode(FILE * out)
{
    int length = stackp;
    int i;

    for(i = 0; i < length; ++i)
        outputCode(out,stringCodeStack[i]);
}

void outputCode(FILE * out,unsigned int code)
{
    static int output_bit_count=0;
    static unsigned long output_bit_buffer=0L;

/*    printf("%d=%c\n",code,(char)(code)); */

    output_bit_buffer |= (unsigned long) code << (32 - BITS- output_bit_count);
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

#if 0
unsigned int inputCode(FILE *input)
{
    unsigned int returnValue;
    static int inputBitCount=0;
    static unsigned long inputBitBuffer=0L;
    unsigned int firstN;
    unsigned int lastN;
    int ch;

/*    printf("Start inputCode\n"); */

    inputBitCount = BITS + inputBitCount;

/*    printf("InputBitCount before start: %d\n",inputBitCount);
    printf("InputBitBuffer before start: %lu\n",inputBitBuffer); */


    while (inputBitCount > 0)
    {
        ch = getc(input);
/*	printf("ch:%d\n",ch); */

        if(ch == EOF){
            returnValue = EOF;
            break;
        }

/*        printf("NEXT BYTE\n");
        printf("inputBitCount:%d\n",inputBitCount);
        printf("Before Inputbuffer:%lu\n",inputBitBuffer);

        printf("ch:%d\n",ch);*/

        if(inputBitCount < 8){
            lastN = lastNBits(ch,4,8);
            firstN = firstNBits(ch,4);
/*            printf("lastN: %d\n",lastN);
            printf("firstN: %d\n",firstN); */

            returnValue = (inputBitBuffer << (BITS - 8)) | lastN;
            inputBitBuffer = firstN;
        } else{
            inputBitBuffer = (inputBitBuffer << (inputBitCount)) | ch;

            if(inputBitCount == 8){
                returnValue = inputBitBuffer;
                inputBitBuffer = 0;
            }
        }

/*        printf("After Inputbuffer:%lu\n",inputBitBuffer); */

        inputBitCount -= 8;
    }

/*    printf("RETURN:%d\n",returnValue); */
    return returnValue;
}

#endif


int find_code(unsigned int charCode)
{
    int index;
    char * str;

    str = constructNewDictionaryEntry(charCode);

    for(index = 0;index < SIZE; ++index){
        if(dictionary[index] == NULL){
            break;
        }
        if(!strcmp(str,dictionary[index])){
            break;
        }
    }
    free(str);

    return index;
}

char * constructNewDictionaryEntry(unsigned int charCode)
{
    char * str;
    int i;

    str = (char * )malloc((stackp + 1) * sizeof(char));

    for(i = 0; i < stackp; ++i){
        str[i] = stringCodeStack[i];
    }

    str[i++] = charCode;
    str[i] = '\0';

    return str;
}
