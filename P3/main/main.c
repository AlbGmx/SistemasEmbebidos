#include "defs.h"

static const char *TAG = "UART TEST";

// Variable Global

void app_main(void) {
	uint8_t playerState = PRE_LOAD, resultado, playAgain;
	char player, guess;
	word_t testWord;
	char *playerGuess = (char *)malloc(BUF_SIZE * sizeof(char));
	initUARTs();
	do {
		testWord = initCharState(testWord, "EstaEsUnaPrueba");

		//  Seleccionar Jugador A o Bsd
		player = selectPlayer();

		putStr(UART_CONSOLE, "\nJugador seleccionado: ");
		putStr(UART_CONSOLE, (player == PLAYER_A) ? "A\n" : "B\n");

		if (player == PLAYER_A) {
			playerState = PLAYING;
			sendGameState(PLAYING);
			clearScreen();
			printGraphic(testWord);
			printWord(testWord);
			while (playerState == PLAYING) {
				putChar(UART_CONSOLE, playerGuess[0]);
				putStr(UART_CONSOLE, "\n\nJugador B, intoduce un intento: ");
				fflush(stdout);
				playerGuess = getLine(UART_ESP);
				if (isValidGuess(playerGuess[0])) {
					printPlayerBGuess(playerGuess);

					if (strlen(playerGuess) > 1) {
						if (strcmp(playerGuess, testWord.guessWord) == 0) {
							playerState = GAME_LOST;
						} else {
							putStr(UART_CONSOLE, "\nPalabra incorrecta\n");
							sendGameState(PLAYING);
							printGraphic(testWord);
							printWord(testWord);
						}
					} else {
						resultado = checkGuess(&testWord, playerGuess[0]);
						if (resultado == LETTER_ALREADY_GUESSED) {
							putStr(UART_CONSOLE, "\nLetra ya utilizada!\n");
						} else if (resultado == LETTER_FOUND) {
							putStr(UART_CONSOLE, "\nNueva letra encontrada!\n");
						} else if (resultado == NO_TRIES_LEFT) {
							putStr(UART_CONSOLE, "\nYa no quedan intentos!\n");
							playerState = GAME_LOST;
						} else if (resultado == WORD_GUESSED) {
							playerState = GAME_WON;
						} else {
							sendGameState(PLAYING);
						}
					}
				}
				printGraphic(testWord);
				printWord(testWord);
			}
			if (playerState == GAME_WON) {
				putStr(UART_CONSOLE, "Ganó el Jugador A\n");
				sendGameState(GAME_LOST);
			} else if (playerState == GAME_LOST) {
				putStr(UART_CONSOLE, "Ganó el Jugador B\n");
				sendGameState(GAME_WON);
			}
			testWord = setAllHits(testWord);
			printGraphic(testWord);
			printWord(testWord);

			playAgain = getChar(UART_ESP);
		} else {

			// Wait for player A to send the game state
			playerState = getChar(UART_ESP);

			while (playerState == PLAYING) {
				putStr(UART_CONSOLE, "Player B, enter a guess: ");
				fflush(stdout);
				putStr(UART_ESP, getLine(UART_CONSOLE));
			}
			if (playerState == GAME_WON) {
				putStr(UART_CONSOLE, "Ganaste\n");
			} else if (playerState == GAME_LOST) {
				putStr(UART_CONSOLE, "Perdiste\n");
			} else {
				putStr(UART_CONSOLE, "Waiting for Player A\n");
			}

			putStr(UART_CONSOLE, "¿Desea jugar de nuevo? (S/N): ");
			fflush(stdout);
			playAgain = (toupper(getChar(UART_CONSOLE)) == 'S') ? 1 : 0;
		}
	} while (playAgain);
}