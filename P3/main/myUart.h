#include "defs.h"

#ifndef MYUART
#define MYUART

void initUARTs(void);

void clearBuffers();
void clearBuffer(int);
void clearScreen(void);

char getChar(int);
void putChar(int, char);
void putStr(int, char *);
void getStr(int, char *);
char *getLine(int);
#endif