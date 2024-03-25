#include "defs.h"

void app_main(void) {
	uint8_t playerState = PRE_LOAD, resultado, playAgain;
	char player;
	word_t testWord;
	char playerGuess;
	initUARTs();
	do {
		testWord = initCharState(testWord, "EstaEsUnaPrueba");

		player = selectPlayer();

		putStr(UART_CONSOLE, "\nJugador seleccionado: ");
		putStr(UART_CONSOLE, (player == PLAYER_A) ? "A\n" : "B\n");

		if (player == PLAYER_A) {
			playerState = PLAYING;
			clearBuffers();
			putStr(UART_CONSOLE, "\n\n\rEspere al Jugador B y presione una tecla cuando ambos estén listos\n");
			getChar(UART_CONSOLE);

			clearScreen();
			printGraphic(testWord);
			printWord(testWord);
			sendGameState(PLAYING);

			while (playerState == PLAYING) {
				clearBuffers();
				clearScreen();
				putStr(UART_CONSOLE, "\n\nEsperando al jugador B:  ");
				playerGuess = getChar(UART_ESP);
				putchar('\n');

				if (isValidGuess(playerGuess)) {
					printPlayerBGuess(playerGuess);
					resultado = checkGuess(&testWord, playerGuess);
					printResultToPlayerB(resultado);
					playerState = updatePlayersStates(resultado);

					printGraphic(testWord);
					printWord(testWord);
				}
			}

			if (playerState == GAME_WON) {
				putStr(UART_CONSOLE, "Ganaste!\n");
				sendGameState(GAME_LOST);
			} else if (playerState == GAME_LOST) {
				putStr(UART_CONSOLE, "Perdiste!\n");
				sendGameState(GAME_WON);
			}

			testWord = setAllHits(testWord);
			printGraphic(testWord);
			printWord(testWord);

			putStr(UART_CONSOLE, "Esperando al Jugador B!\n");
			playAgain = getChar(UART_ESP);
		} else {

			putStr(UART_CONSOLE, "\n\nEsperando al Jugador A: ");
			playerState = getValidStateFromPlayerA();
			clearScreen();
			while (playerState == PLAYING) {
				putStr(UART_CONSOLE, "\nPlayer B, introduce un intento: ");
				playerState = sendGuessToPlayerA();
				if (playerState == GAME_WON) {
					putStr(UART_CONSOLE, "\nGanaste!\n");
				} else if (playerState == GAME_LOST) {
					putStr(UART_CONSOLE, "\nPerdiste!\n");
				} else {
					playerState = PLAYING;
				}
			}

			putStr(UART_CONSOLE, "\n¿Desea jugar de nuevo? (S/N): ");
			playAgain = (toupper(getChar(UART_CONSOLE)) == 'S') ? 1 : 0;
			sendGameState((playAgain) ? PRE_LOAD : GAME_WON);
		}
	} while (playAgain);
}