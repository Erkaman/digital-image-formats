#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../common.h"
#include "util.h"

void printHelp(void);

void verbosePrint(const char * format, ...);
void debugPrint(const char * format, ...);

#define DEBUG 1

/* compression */

void lzw_compress(FILE * in,FILE * out);

void outputCode(unsigned int code,FILE * out);

int findMatch(unsigned int hash_prefix,unsigned int hash_character);

/* decompression */

void lzw_decompress(FILE * in,FILE * out);


void translateCode(unsigned int newCode);
char printString(FILE * out);

unsigned int inputCode(FILE *input);



typedef struct {
    unsigned int stringCode;
    unsigned int characterCode;
} tableEntry;

tableEntry * stringTable;
int * codeValues;

char stringCodeStack[40000];
int stackp;

int verbose;

unsigned int codeSize;
unsigned int tableSize;

unsigned int  hashingShift;

unsigned int maxValue;
unsigned int maxCode;

unsigned int dictionaryIndex;

int main(int argc, char *argv[])
{
    int decompress = 0;
    FILE * in;
    FILE * out;
    char * outFile;
    char * inFile;
    char extension[5];

    codeSize = 12;

    verbose = 0;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else{

        ++argv;

        while(--argc != 1){

            debugPrint("Argv:%s\n",*argv);

            if(!strcmp("--help",*argv)){
                printHelp();
                return 0;
            }
            else if(!strcmp("-d",*argv))
                decompress = 1;
            else if(!strcmp("-v",*argv))
                verbose = 1;
            else if(!strncmp("-bs=",*argv,4)){
                (*argv)+= 4;
                debugPrint("-bs= %s\n",*argv);

                codeSize = atoi(*argv);

                if(!(codeSize >= 9 && codeSize<=15)){
                    printf("Code length specifier is invalid, defaulting to 12.\n");
                    codeSize = 12;

                }
                verbose = 1;
            }

            ++argv;

        }

        if(codeSize == 15)
            tableSize = 33941;
        else if(codeSize == 14)
            tableSize =  18041;
        else if(codeSize == 13)
            tableSize =  9029;
        else if(codeSize <= 12)
            tableSize = 5021;

        hashingShift = (codeSize-8);
        maxValue = (1 << codeSize) - 1;
        maxCode =  maxValue - 1 ;


        stringTable = (tableEntry *)malloc(sizeof(tableEntry) * tableSize);
        codeValues = (int *)malloc(sizeof(int) * tableSize);

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
    printf("  -v\tVerbose output..\n");

}

void translateCode(unsigned int newCode)
{
    tableEntry entry;

    debugPrint("translateCode\n");
    debugPrint("newCode:%d\n",newCode);

    entry = stringTable[newCode];

    while(1){

        debugPrint("characterCode:%d\n",entry.characterCode);

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
    while (newCode != maxValue){

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
        if(dictionaryIndex <= maxCode){

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
    unsigned int nextCode;
    int index;

    for(nextCode = 0; nextCode < tableSize; ++nextCode)
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
            if(nextCode <= maxCode){

                verbosePrint("Added new dictionary entry:%d {%d = %c,%d = %c}\n",
                             nextCode,
                             stringCode,
                             stringCode,
                             charCode,
                             charCode);

                stringTable[index].characterCode = charCode;
                stringTable[index].stringCode = stringCode;

                codeValues[index] = nextCode++;
            }

            stringCode = charCode;
        }

        charCode = getc(in);
    }

    outputCode(stringCode,out);
    outputCode(maxValue,out);
    outputCode(0,out);
}

void outputCode(unsigned int code,FILE * out)
{
    static int output_bit_count=0;
    static unsigned int output_bit_buffer=0L;

    verbosePrint("Outputted code: %d=%c\n",code,(char)(code));

    output_bit_buffer |= (unsigned int) code << (32 - codeSize- output_bit_count);
    output_bit_count += codeSize;
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
    return_value=input_bit_buffer >> (32-codeSize);
    input_bit_buffer <<= codeSize;
    input_bit_count -= codeSize;
    return(return_value);
}

int findMatch(unsigned int stringCode,unsigned int charCode)
{
    int index;
    int offset;

    index = (charCode << hashingShift) ^ stringCode;
    if (index == 0)
        offset = 1;
    else
        offset = tableSize - index;
    while (1)
    {
        if (codeValues[index] == -1)
            return index;

        if (stringTable[index].stringCode == stringCode &&
            stringTable[index].characterCode == charCode)
            return index;

        index -= offset;

        if (index < 0)
            index += tableSize;
    }
}

void verbosePrint(const char * format, ...)
{
    va_list vl;

    if(verbose){
        va_start(vl, format);
        vprintf(format, vl);
        va_end(vl);
    }
}

void debugPrint(const char * format, ...)
{
    va_list vl;

    if(DEBUG){
        va_start(vl, format);
        vprintf(format, vl);
        va_end(vl);
    }

}
