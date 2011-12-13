#ifndef _PRINT_FUNCS_H_
#define _PRINT_FUNCS_H_

int verbose;

void printByte(void * byte);

void verbosePrint(const char * format, ...);

void printWarning(const char * format, ...);
void printError(const char * format, ...);

#endif /* _PRINT_FUNCS_H_ */
