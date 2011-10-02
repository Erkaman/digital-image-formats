#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common.h"

void printHelp(void);
void lzw_compress(FILE * in,FILE * out);
void lzw_decompress(FILE * in,FILE * out);

void outputCode(FILE * out,unsigned int code);

void outputStringCode(FILE * out);

#define BITS 12
#define SIZE 4096 /* or sumthin */
#define MAX_VALUE (1 << BITS) - 1 /* Note that MS-DOS machines need to   */
#define MAX_CODE MAX_VALUE - 1    /* compile their code in large model if*/

char ** dictionary;

char stringCodeStack[40000];
int stackp;
int dictionarySize;


char * strAppend ( char * str, const char * appendee );

char * strncpyReverse( char * destination, const char * source, size_t num );

int find_code(unsigned int charCode);

char * constructNewDictionaryEntry(unsigned int charCode);
void addToDictionary (int index,unsigned int charCode);
void printStringCode(unsigned int stringCode);

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

        inFile = *argv;
        in = fopen(inFile,"rb");
        assertFileOpened(in);

        if(decompress){
            strncpyReverse(extension,*argv,4);

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

            printf("Dictionary\n");
            for(i = 0;i < 11; ++i){
                printf("%d : %s\n",i + 256,dictionary[i]);
            }
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

void lzw_decompress(FILE * in,FILE * out)
{
    in = in;
    out = out;
}

void lzw_compress(FILE * in,FILE * out)
{
    unsigned int charCode;
    unsigned int i;
    int index;
    int formerIndex;

    dictionarySize = 0;

    for(i = 0; i < SIZE; ++i)
        dictionary[i] = NULL;

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
            if(dictionarySize < MAX_VALUE){
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

/*    if(dictionary[code] != NULL){
      code += 256;
      }*/

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

char * strAppend ( char * str, const char * appendee )
{
    char * appended;

    appended = (char *)malloc(strlen(str) + strlen(appendee));
    strcpy(appended,str);
    strcat(appended,appendee);

    return appended;
}

char * strncpyReverse( char * destination, const char * source, size_t num )
{
    int skip;

    skip = strlen(source) - num;

    source += skip;

    strncpy(destination,source,num + 1);

    return destination;
}
