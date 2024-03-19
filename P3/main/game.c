#include "game.h"
#include <stdint.h>
#include "myUart.h"
#include <ctype.h>

void initCharState(word_t word) {
	uint8_t i;
	for (i = 0; i < word.len; i++) {
		word.guessWord[i] = HIDDEN_CHAR;
	}
}

uint8_t getPlayer() {
	char c;
	while (1) {
		printf("Selecciona el jugador (A o B): ");
        fflush(stdout);
		c = getChar(UART_MONITOR);
		if (toupper(c) == 'A') return PLAYER_A;
		if (toupper(c) == 'B') return PLAYER_B;

		printf("\nJugador invalido");
	}
}

void printGraphic(word_t word ) {
    printf("\n\n");
    printf("  +---+\n");
    printf("  |   |\n");
    printf("  %c   |\n", (word.misses > 0) ? 'O' : ' ');
    printf(" %c%c%c  |\n", (word.misses > 2) ? '/' : ' ', (word.misses > 1) ? '|' : ' ', (word.misses > 3) ? '\\' : ' ');
    printf(" %c %c |\n", (word.misses > 4) ? '/' : ' ', (word.misses > 5) ? '\\' : ' ');
    printf("      |\n");
    printf("=========\n");
}

void printWord(word_t word) {
    uint8_t i;
    char c;
    for (i = 0; i < word.len; i++) {
        c = (word.hits[i] == FOUND_CHAR) ? word.guessWord[i] : '_';
        printf("%c ", c);
    }
}

uint8_t isWordGuessed(word_t word) {
    uint8_t i;
    for (i = 0; i < word.len; i++) {
        if (word.hits[i] == HIDDEN_CHAR) return 0;
    }
    return 1;
}

uint8_t checkGuess(word_t word, char guess) {
	uint8_t i;
	uint8_t found = 0;
	for (i = 0; i < word.len; i++) {
		if (toupper(word.guessWord[i]) == toupper(guess)) {
			word.hits[i] = FOUND_CHAR;
            found = 1;
		}
	}
	if (found == 0) {
        word.misses++; 
    }
	return word.misses>=MAX_MISSES ? 0 : 1;
}