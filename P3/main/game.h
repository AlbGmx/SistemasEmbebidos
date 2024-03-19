#include "defs.h"

#ifndef GAME
#define GAME
uint8_t getPlayer();
void printGraphic(word_t word);
void printWord(word_t word);
void initCharState(word_t word);
uint8_t checkGuess(word_t word, char c);
#endif
