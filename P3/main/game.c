#include "defs.h"

char *TAG = "GAME";

word_t initCharState(word_t word, char *guessWord) {
	uint8_t i;
	word.guessWord = guessWord;
	word.len = strlen(guessWord);
	for (i = 0; i < word.len; i++) {
		word.hits[i] = HIDDEN_CHAR;
	}
	word.misses = 0;
	return word;
}

uint8_t selectPlayer() {
	char *c;
	printf("\nSelecciona el jugador (A o B): ");
	while (1) {
		fflush(stdout);
		c = getLine(UART_CONSOLE);
		if (toupper(*c) == 'A') return PLAYER_A;
		if (toupper(*c) == 'B') return PLAYER_B;

		printf("\nJugador invalido, intenta de nuevo: ");
	}
}

void printGraphic(word_t word) {

	printf("\n\n");
	printf("\t\t  +---+\n");
	printf("\t\t  |   |\n");
	printf("\t\t  %c   |\n", (word.misses > 0) ? 'O' : ' ');
	printf("\t\t %c%c%c  |\n", (word.misses > 2) ? '/' : ' ', (word.misses > 1) ? '|' : ' ',
			 (word.misses > 3) ? '\\' : ' ');
	printf("\t\t %c %c  |\n", (word.misses > 4) ? '/' : ' ', (word.misses > 5) ? '\\' : ' ');
	printf("\t\t      |\n");
	printf("\t\t=========\n");
}

void printWord(word_t word) {
	uint8_t i;
	char c;
	printf("\n\t");
	for (i = 0; i < word.len; i++) {
		printf("%c ", (word.hits[i] == FOUND_CHAR) ? word.guessWord[i] : '_');
	}
	printf("\n");
}

uint8_t isWordGuessed(word_t word) {
	uint8_t i;
	for (i = 0; i < word.len; i++) {
		if (word.hits[i] == HIDDEN_CHAR) return WORD_NOT_GUESSED;
	}
	return WORD_GUESSED;
}

uint8_t checkGuess(word_t *word, char guess) {
	uint8_t i;
	uint8_t found = 0;

	for (i = 0; i < word->misses; i++) {
		if (toupper(word->lettersGuessed[i]) == toupper(guess)) {
			return LETTER_ALREADY_GUESSED;
		}
	}

	for (i = 0; i < word->len; i++) {
		if (toupper(word->guessWord[i]) == toupper(guess)) {
			if (word->hits[i] == FOUND_CHAR) {
				word->misses++;
				return LETTER_ALREADY_GUESSED;
			}
			word->hits[i] = FOUND_CHAR;
			found			  = 1;
		}
	}
	if (found) {
		if (isWordGuessed(*word)) return WORD_GUESSED;
		return LETTER_FOUND;
	} else {
		word->misses++;
	}
	return word->misses >= MAX_MISSES ? NO_TRIES_LEFT : NO_CHANGE;
}

word_t setAllHits(word_t word) {
	uint8_t i;
	for (i = 0; i < word.len; i++) {
		word.hits[i] = FOUND_CHAR;
	}
	return word;
}

void sendGameState(uint8_t state) { putChar(UART_ESP, state); }

void printPlayerBGuess(char *playerGuess) {
	if (isValidGuess(playerGuess[0])) {
		putStr(UART_CONSOLE, "\n\nJugador B, adivinó ");
		putStr(UART_CONSOLE, (strlen(playerGuess) > 1) ? "la palabra: \'" : "la letra: \'");
		putStr(UART_CONSOLE, playerGuess);
		putStr(UART_CONSOLE, "\'\n");
	}
}

uint8_t isValidGuess(char playerGuess) { return isascii(toupper(playerGuess)); }