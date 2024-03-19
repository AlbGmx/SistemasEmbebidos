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
	uint8_t player;
	char c;
	initUARTs();
	initCharState(testWord);
	// Seleccionar Jugador A o B
	player = getPlayer();
	ESP_LOGI(TAG, "Jugador seleccionado: %c", player);

	if(player == PLAYER_A){
		while(testWord.misses < MAX_MISSES && !isWordGuessed(testWord)){
			c = getChar(UART_PLAYERS);
			
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
