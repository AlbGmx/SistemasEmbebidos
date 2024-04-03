#ifndef DEFS
#define DEFS

// Public libraries
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"

// GAME
#define MAX_MISSES 6
#define PLAYER_A 0
#define PLAYER_B 1

#define BUF_SIZE 24

// UART
#define UART_CONSOLE UART_NUM_0
#define UART_ESP UART_NUM_2
#define UART_BUFFER 1024

#define UART_CONSOLE_RX_PIN (3)
#define UART_CONSOLE_TX_PIN (1)
#define UART_PLAYER_B_RX_PIN (16)
#define UART_PLAYER_B_TX_PIN (17)
#define MY_UART_RX 2
#define MY_UART_TX 4

#define ONE_BYTE 1
#define ASCII_PRINTABLE_START 32
#define ASCII_PRINTABLE_END 126
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'
#define NULL_TERMINATOR '\0'

enum gameState {
   PLAYING = '=', // Random values to reduce false positives from trash data
   GAME_WON,
   GAME_LOST,
   PRE_LOAD,
};

enum guessState {
   WORD_NOT_GUESSED,
   WORD_GUESSED,
   LETTER_FOUND,
   LETTER_ALREADY_GUESSED,
   NO_CHANGE,
   NO_TRIES_LEFT,
};

enum wordState {
   END_OF_ARRAY,
   HIDDEN_CHAR,
   FOUND_CHAR,
};

typedef struct {
   char *guessWord;
   uint8_t hits[BUF_SIZE];
   uint8_t len;
   uint8_t misses;
   uint8_t guesses;
   uint8_t lettersTried[MAX_MISSES];
} word_t;

// Private libraries
#include "game.h"
#include "myUart.h"

#endif