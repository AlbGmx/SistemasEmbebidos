#ifndef MYUART
#define MYUART
#include "defs.h"

void initUARTs(void);
char getChar(int);
void putChar(int, char);
void putStr(int, char *);
void getStr(int, char *);
char *getLine(int);

#endif