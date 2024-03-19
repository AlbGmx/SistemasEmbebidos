#include "driver/uart.h"

// GAME
#define PLAYER_A 0
#define PLAYER_B 1

#define BUF_SIZE (24)
#define UART_RX_PIN (3)
#define UART_TX_PIN (1)

// UART
#define UART_MONITOR UART_NUM_0
#define UART_PLAYERS UART_NUM_2
#define UART_BUFFER 100

#define UART_RX_PIN_2 (16)
#define UART_TX_PIN_2 (17)

#define ONE_BYTE 1
#define ASCII_PRINTABLE_START 32
#define ASCII_PRINTABLE_END 126
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'
#define NULL_TERMINATOR '\0'

#define MAX_MISSES 6

enum {
   PRE_LOAD = 0,
   PLAYING,
   GAME_WON,
   GAME_LOST
}

enum {
  END_OF_ARRAY = 0,
  HIDDEN_CHAR,
  FOUND_CHAR,
};

typedef struct {
  char *guessWord;
  uint8_t hits[BUF_SIZE];
  uint8_t len;
  uint8_t misses;
} word_t;

