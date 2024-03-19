#include "defs.h"

#ifndef GAME
#define GAME
uint8_t selectPlayer();
void printGraphic(word_t word);
void printWord(word_t word);
word_t initCharState(word_t word, char *guessWord);
uint8_t checkGuess(word_t *word, char c);
uint8_t isWordGuessed(word_t word);
word_t setAllHits(word_t word);
void sendGameState(uint8_t state);
void printPlayerBGuess(char *playerGuess);
uint8_t isValidGuess(char playerGuess);
#endif
