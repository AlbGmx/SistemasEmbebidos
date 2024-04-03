#include "defs.h"

#ifndef GAME
#define GAME

uint8_t selectPlayer();
word_t setAllHits(word_t);
word_t initCharState(word_t, char *);

// Game Logic
void sendGameState(uint8_t);
char sendGuessToPlayerA();
char getValidStateFromPlayerA();
uint8_t isValidGuess(char);
uint8_t isWordGuessed(word_t);
uint8_t checkGuess(word_t *, char);
uint8_t updatePlayersStates(uint8_t);
char *getWordToGuess();

// Prints
void printWord(word_t);
void printGraphic(word_t);
void printPlayerBGuess(char);
void printResultToPlayerB(uint8_t);
#endif
