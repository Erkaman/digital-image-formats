#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../common.h"
#include "util.h"

/* FIX THE DAMN COMMAD LINE PARSER: THE LINE ./lzw --help doesn't work!!!! */

void printHelp(void);
void verbosePrint(const char * format, ...);
void debugPrint(const char * format, ...);

/* Set this macro to 1 if you want helpful debug messages */
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


int main(int argc, char *argv[])
{
    int decompress = 0;
    FILE * in;
    FILE * out;
    char * outFile;
    char * inFile;
    char extension[5];

/*    codeSize = 9;
    out = fopen("out","wb");

    outputCode(4,out);
    outputCode(3,out);
    outputCode(0,out);

    fclose(out);

    in = fopen("out","rb");

    printf("%d\n",inputCode(in));
    printf("%d\n",inputCode(in));

    exit(0);*/

    codeSize = 12;
    verbose = 0;

    if(argc == 1){
        printf("No file was specified.\n");
        printf("Try --help for more information.\n");
    } else{

        ++argv;
        --argc;

        while(1){

            if(!strcmp("--help",*argv)){
                printHelp();
                return 0;
            }
            else if(!strcmp("-d",*argv))
                decompress = 1;
            else if(!strcmp("-v",*argv))
                verbose = 1;
            else if(!strncmp("-cs=",*argv,4)){
                (*argv)+= 4;

                codeSize = atoi(*argv);

                if(!(codeSize >= 9 && codeSize<=15)){
                    printf("Code length specifier is invalid, defaulting to 12.\n");
                    codeSize = 12;

                }
            } else if(argc == 1){
                break;
            }
            ++argv;
            --argc;
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

        free(stringTable);
        free(codeValues);
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
    printf("  -v\tVerbose output.\n");
    printf("  -cs=\tSet the outputted code sizes.\n");

}

void translateCode(unsigned int newCode)
{
    tableEntry entry;

    entry = stringTable[newCode];

    while(1){

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
        verbosePrint("Outputted translated character:%d\n",stringCodeStack[stackp-1]);
        putc(stringCodeStack[--stackp],out);
    }

    return returnValue;
}

void lzw_decompress(FILE * in,FILE * out)
{
    unsigned int oldCode;
    unsigned int newCode;
    char character;
    unsigned int nextCode;


    for(nextCode = 0; nextCode < 256; ++nextCode){
        stringTable[nextCode].characterCode = nextCode;
        stringTable[nextCode].stringCode = (unsigned int) -1;
    }

    nextCode = 256;

    oldCode = inputCode(in);

    verbosePrint("Outputted translated character:%d\n",oldCode);
    putc(oldCode,out);

    character = oldCode;

    newCode = inputCode(in);

    while (newCode != maxValue){

        /*if it is not in the translation table. */
        if(!(newCode < nextCode)){
            stringCodeStack[stackp++] = character;
            translateCode(oldCode);

        } else
            translateCode(newCode);

        character = printString(out);

        /* add it the table */
        if(nextCode <= maxCode){

            stringTable[nextCode].stringCode = oldCode;
            stringTable[nextCode].characterCode = character;

            verbosePrint("Added new dictionary entry:%d {%d = %c,%d = %c}\n",
                         nextCode,
                         oldCode,
                         oldCode,
                         character,
                         character);


            nextCode++;
        }

        oldCode = newCode;
        newCode = inputCode(in);
    }
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
/*    The number of remaining bits to be written before a whole new byte is outputted. */
    static int remainingPacketBits = 8;
    static BYTE packet = 0;

/*    the number of remaing bits in the current code. */
    int remainingCodeBits;

    static int shift = 0;

    remainingCodeBits = codeSize;

    /* while all bits haven't yet been read. */
    while(remainingCodeBits > 0){

	/* if the number of bits to be read is less than the current input value */
        if(remainingPacketBits < remainingCodeBits){
            /* write what can be written*/

            packet |=
                (firstNBits(code,remainingPacketBits) << shift);

            putc(packet,out);

            remainingCodeBits -= remainingPacketBits;

            code >>= remainingPacketBits;

	    /* reset the packet */
	    remainingPacketBits = 8;
	    packet = 0;
            shift = 0;
        }else{
	    /* if the number of bits remaining to be read is less than the
	     bits in the current input value. */

            /* remainingPacketBits >= remainingCodeBits */
	    code = firstNBits(code,remainingCodeBits);
            packet |= code << shift;

            shift += remainingCodeBits;
            remainingPacketBits -=  remainingCodeBits;

            remainingCodeBits = 0;
        }

    }
}

unsigned int inputCode(FILE * input)
{
    unsigned int code;
    int remainingCodeBits;
    int shift;

    static int readFirstCharacter = 0;
    static int packet;
    static int remainingPacketBits = 8;

    remainingCodeBits = codeSize;
    code = 0;
    shift = 0;

    /* read the first character */
    if(!readFirstCharacter){
        packet = (BYTE)getc(input);
        readFirstCharacter = 1;
    }

    /* While an entire code hasn't yet been read.  */
    while(remainingCodeBits > 0){
	/* If the current packet doesn't contain enough bits
          to fill the current code. */
        if(remainingPacketBits < remainingCodeBits){

            code |=
                (firstNBits(packet,remainingPacketBits) << shift);

            shift += remainingPacketBits;
            remainingCodeBits -= remainingPacketBits;

	    /* read in a new packet */
            packet = (BYTE)getc(input);
            remainingPacketBits = 8;
        }else{
	    /* If the number of bits remaining to be read in the code
	     * is less than or equal to the number of bits in the
	     * current packet. */

	    /* read as many bits needed from the packet. */
            code |=
                (firstNBits(packet,remainingCodeBits) << shift);

	    /* Right shift away the bits that were written to the code.*/
            packet >>= remainingCodeBits;
            remainingPacketBits -= remainingCodeBits;

	    /* The entire code has been read, stop the loop. */
            remainingCodeBits = 0;
        }
    }

    verbosePrint("Inputted code: %d=%c\n",code,(char)(code));

    return code;
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
