#ifndef CONSOLE_H
#define CONSOLE_H

#include "driver/uart.h"

void clearScreen();
void hideCursor();
void showCursor();
void initializeUart();
void delayMillis(int);
void goToXY(int, int);

#endif
