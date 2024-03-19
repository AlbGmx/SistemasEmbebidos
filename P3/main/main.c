#include "defs.h"

static const char *TAG = "UART TEST";

word_t testWord = {
	 .guessWord = "hola",
	 .len			= 4,
	 .hits		= {0},
	 .misses		= 0,
};

void app_main(void) {
	uint8_t player;
	//	uint8_t rslt;
	//	char c;
	initUARTs();
	// initCharState(testWord);
	//  Seleccionar Jugador A o B
	while (1) {
		player = getPlayer();
		ESP_LOGI(TAG, "Player: %s", (player == PLAYER_A) ? "A" : "B");
		putStr(UART_CONSOLE, "Player selected");
		putStr(UART_PLAYER_B, "Print in player B UART\n");
	}

	/*
		if (player == PLAYER_A) {

			// Send start instruction to player 2 via UART_2
			putStr(UART_PLAYER_B, "Start, Player ");


					while (testWord.misses < MAX_MISSES && !isWordGuessed(testWord)) {
						c	  = getChar(UART_PLAYER_B);
						rslt = checkChar(testWord, c);
						printGraphic(testWord);
						printWord(testWord);
						if (rslt == 0) {
							// Perdiox
							ESP_LOGI(TAG, "Ganaste");
							// send message to player 2 via UART_2
							putStr(UART_PLAYER_B, "Perdiste\n");
							break;
						}
					}
					if (isWordGuessed(testWord)) {
						// Gano
						ESP_LOGI(TAG, "Ganó el Player 2");
						// send message to player 2 via UART_2
						putStr(UART_PLAYER_B, *"Ganaste\n");
					}

				}
	}
	else {
*/
}
