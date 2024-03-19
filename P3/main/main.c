#include <string.h>
#include <inttypes.h>
#include "esp_log.h"
#include "myUart.h"
#include "game.h"
#include "defs.h"

static const char *TAG = "UART TEST";

word_t testWord = {
	 .guessWord = "hola",
	 .len			= 4,
	 .hits[BUF_SIZE] = {0},
	 .misses		= 0,
};

void app_main(void) {
	uint8_t player,rslt;
	char c;
	initUARTs();
	
	initCharState(testWord);
	// Seleccionar Jugador A o B
	player = getPlayer();
	ESP_LOGI(TAG, "Jugador seleccionado: %c", player);

	if(player == PLAYER_A){
		//Send start instruction to player 2 via UART_2
		putStr(UART_PLAYERS, "Start, Player 2\n");

		while(testWord.misses < MAX_MISSES && !isWordGuessed(testWord)){
			c = getChar(UART_PLAYERS);
			rslt = checkChar(testWord, c);
			printGraphic(testWord);
			printWord(testWord);
			if(rslt == 0){
				// Perdio
				ESP_LOGI(TAG, "Ganaste");
				//send message to player 2 via UART_2
				putStr(UART_PLAYERS, "Perdiste\n");
				break;
			}
			
		}
		if(isWordGuessed(testWord)){
			//Gano
			ESP_LOGI(TAG, "Ganó el Player 2");
			//send message to player 2 via UART_2
			putStr(UART_PLAYERS, "Ganaste\n");
		}
		
	}else{
      while(){
         
         
         
      }
		
	}
	/*
	Spinlock UART para esperar 1 char del Jugador 2
	getChar(){
		//Spinlock
		//Espera a que el jugador 2 envie un char
		//Si el jugador 2 envia un char, lo guarda en una variable
		//Desbloquea el spinlock

	}

	Verificar si el char existe en la palabra

	Actualiza el estado del juego e imprime
	*/
}
