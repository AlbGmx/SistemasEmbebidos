#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <string.h>

#define BUF_SIZE (1024)
#define UART_RX_PIN (3)
#define UART_TX_PIN (1)

#define UART_MONITOR UART_NUM_0
#define UART_PLAYERS UART_NUM_2
#define MAX_BUFFER 100

#define UART_RX_PIN_2 (16)
#define UART_TX_PIN_2 (17)

#define ONE_BYTE 1
#define ASCII_PRINTABLE_START 32
#define ASCII_PRINTABLE_END 126
#define NEWLINE '\n'
#define CARRIAGE_RETURN '\r'
#define NULL_TERMINATOR '\0'

static void initUARTs() {
	uart_config_t uart_config = {
		 .baud_rate	 = 115200,
		 .data_bits	 = UART_DATA_8_BITS,
		 .parity		 = UART_PARITY_DISABLE,
		 .stop_bits	 = UART_STOP_BITS_1,
		 .flow_ctrl	 = UART_HW_FLOWCTRL_DISABLE,
		 .source_clk = UART_SCLK_DEFAULT,
	};
	// Configure UART parameters
	ESP_ERROR_CHECK(uart_param_config(UART_MONITOR, &uart_config));
	ESP_ERROR_CHECK(uart_param_config(UART_PLAYERS, &uart_config));

	// Set UART pins
	ESP_ERROR_CHECK(uart_set_pin(UART_MONITOR, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
	ESP_ERROR_CHECK(uart_set_pin(UART_PLAYERS, UART_TX_PIN_2, UART_RX_PIN_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

	// Setup UART buffered IO with event queue
	QueueHandle_t uart_queue;
	// Install UART driver using an event queue here
	ESP_ERROR_CHECK(uart_driver_install(UART_MONITOR, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0));
	ESP_ERROR_CHECK(uart_driver_install(UART_PLAYERS, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0));
}

bool UART_kbHit(void) { return (uart_get_buffered_data_len(UART_MONITOR, NULL) > 0); }

char UART_getChar(void) {
	uint8_t data;
	int len = uart_read_bytes(UART_MONITOR, &data, ONE_BYTE, 20 / portTICK_PERIOD_MS);
	return data;
}

void UART_putChar(char c) { uart_write_bytes(UART_MONITOR, &c, ONE_BYTE); }

void UART_puts(char *str) {
	while (*str != '\0')
		UART_putChar(*str++);
}

void UART_gets(char *str) {
	size_t i = 0;
	char c;

	while (uart_read_bytes(UART_MONITOR, (uint8_t *)&c, 1, portMAX_DELAY) != CARRIAGE_RETURN) {
		if (c >= ASCII_PRINTABLE_START && c <= ASCII_PRINTABLE_END && i < BUF_SIZE - 1) str[i++] = c;
	}
	str[i] = NULL_TERMINATOR;
}

char getChar() {
	uint8_t data = 0;
	while (1) {
		int len = uart_read_bytes(UART_MONITOR, &data, 1, portMAX_DELAY);
		if (len == 1) {
			return (char)data;
		}
	}
}
char *getline() {
	static char buffer[MAX_BUFFER];
	memset(buffer, 0, sizeof(buffer)); // Clear buffer
	int index = 0;
	while (1) {
		char c = getChar();
		if (c == '\n' || c == '\r') {
			printf("\n"); // Print newline
			break;
		} else if (c == '\b' || c == 127) { // Backspace or delete
			if (index > 0) {
				index--;
				printf("\b \b"); // Move cursor back, erase character, move cursor back again
			}
		} else if (index < MAX_BUFFER - 1) {
			buffer[index++] = c;
			printf("%c", c); // Echo character
		}
	}
	fflush(stdout);
	buffer[index] = '\0'; // Null-terminate the string
	return buffer;
}
