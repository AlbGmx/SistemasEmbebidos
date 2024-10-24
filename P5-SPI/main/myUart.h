#ifndef MYUART
#define MYUART
#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define UART_CONSOLE UART_NUM_0
#define UART_ESP UART_NUM_2
#define UART_CONSOLE_TX_PIN GPIO_NUM_1
#define UART_CONSOLE_RX_PIN GPIO_NUM_3
#define UART_ESP_TX_PIN GPIO_NUM_17
#define UART_ESP_RX_PIN GPIO_NUM_16
#define UART_BAUD_RATE 115200
#define BUFFER_SIZE 2048
#define GOTOXY_BUFFER_SIZE 10

// ASCII Constants
#define ONE_BYTE 1
#define ASCII_PRINTABLE_START 32
#define ASCII_PRINTABLE_END 126
#define ASCII_NUMBERS_START '0'
#define ASCII_NUMBERS_END '9'
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'
#define NULL_TERMINATOR '\0'

void init_UARTs(void);
void clear_screen(uint8_t);
void go_to_XY(uint8_t, uint8_t, uint8_t);
char get_char(uint8_t);
void put_char(uint8_t, char);
void put_str(uint8_t, char *);
uint16_t get_str(uint8_t, char *);
char *get_line(uint8_t);
void hide_cursor(uint8_t);
void show_cursor(uint8_t);

#endif