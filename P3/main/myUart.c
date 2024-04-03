#include "defs.h"

void initUARTs() {
   uart_config_t uart_config = {
       .baud_rate = 115200,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
       .source_clk = UART_SCLK_DEFAULT,
   };
   // Configure UART parameters
   ESP_ERROR_CHECK(uart_param_config(UART_CONSOLE, &uart_config));
   ESP_ERROR_CHECK(uart_param_config(UART_ESP, &uart_config));

   // Set UART pins
   ESP_ERROR_CHECK(
       uart_set_pin(UART_CONSOLE, UART_CONSOLE_TX_PIN, UART_CONSOLE_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
   ESP_ERROR_CHECK(
       uart_set_pin(UART_ESP, UART_PLAYER_B_TX_PIN, UART_PLAYER_B_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

   ESP_ERROR_CHECK(uart_driver_install(UART_CONSOLE, UART_BUFFER * 2, 0, 0, NULL, 0));
   ESP_ERROR_CHECK(uart_driver_install(UART_ESP, UART_BUFFER * 2, 0, 0, NULL, 0));
}

void putChar(int uartPort, char c) { uart_write_bytes(uartPort, &c, ONE_BYTE); }

void putStr(int uartPort, char *str) {
   while (*str) {
      putChar(uartPort, *str++);
   }
}

void getStr(int uartPort, char *str) {
   size_t i = 0;
   char c;

   while (uart_read_bytes(uartPort, (uint8_t *)&c, ONE_BYTE, portMAX_DELAY) != CARRIAGE_RETURN)
      if (c >= ASCII_PRINTABLE_START && c <= ASCII_PRINTABLE_END && i < BUF_SIZE - ONE_BYTE) str[i++] = c;

   str[i] = NULL_TERMINATOR;
}

char getChar(int uartPort) {
   uint8_t data = 0;
   while (1) {
      int len = uart_read_bytes(uartPort, &data, ONE_BYTE, portMAX_DELAY);
      if (len == 1) return (char)data;
   }
}

char *getLine(int uartPort) {
   static char buffer[BUF_SIZE];
   memset(buffer, 0, sizeof(buffer)); // Clear buffer
   int index = 0;
   while (1) {
      char c = getChar(uartPort);
      if (c == '\n' || c == '\r') {
         printf("\n"); // Print newline
         break;
      } else if (c == '\b' || c == 127) { // Backspace or delete
         if (index > 0) {
            index--;
            printf("\b \b"); // Move cursor back, erase character, move cursor back again
         }
      } else if (index < BUF_SIZE - 1) {
         buffer[index++] = c;
         printf("%c", c); // Echo character
      }
      fflush(stdout);
   }
   printf("\n");         // Print newline
   buffer[index] = '\0'; // Null-terminate the string
   return buffer;
}

void clearScreen() { putStr(UART_CONSOLE, "\033[2J\033[H"); }