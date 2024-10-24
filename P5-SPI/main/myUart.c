#include "myUart.h"

static const char *TAG = "My Uart Library";
void init_UARTs() {
   uart_config_t uart_config = {
       .baud_rate = UART_BAUD_RATE,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_1,
       .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
       .source_clk = UART_SCLK_DEFAULT,
   };
   // Set UART_CONSOLE
   ESP_ERROR_CHECK(uart_driver_install(UART_CONSOLE, BUFFER_SIZE, 0, 0, NULL, 0));
   ESP_ERROR_CHECK(uart_param_config(UART_CONSOLE, &uart_config));
   ESP_ERROR_CHECK(
       uart_set_pin(UART_CONSOLE, UART_CONSOLE_TX_PIN, UART_CONSOLE_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
   ESP_LOGI(TAG, "UART setup complete");
}

void put_char(uint8_t uartPort, char c) { uart_write_bytes(uartPort, &c, ONE_BYTE); }

void put_str(uint8_t uartPort, char *str) {
   while (*str) {
      put_char(uartPort, *str++);
   }
}

// uint16_t get_str(uint8_t uartPort, char *str) {
//    uint16_t size = 0;
//    char c;

//    while (uart_read_bytes(uartPort, (uint8_t *)&c, ONE_BYTE, portMAX_DELAY) != CARRIAGE_RETURN)
//       if (c >= ASCII_PRINTABLE_START && c <= ASCII_PRINTABLE_END && size < BUFFER_SIZE - ONE_BYTE) str[size++] = c;

//    str[size] = NULL_TERMINATOR;
//    return size;
// }

char get_char(uint8_t uartPort) {
   uint8_t data = 0;
   while (1) {
      uint16_t len = uart_read_bytes(uartPort, &data, ONE_BYTE, portMAX_DELAY);
      if (len == 1) return (char)data;
   }
}

char *get_line(uint8_t uartPort) {
   static char buffer[BUFFER_SIZE];
   memset(buffer, 0, sizeof(buffer));
   uint16_t index = 0;
   while (true) {
      char c = get_char(uartPort);
      if (c == '\n' || c == '\r') {
         break;
      } else if (c == '\b' || c == 127) {
         if (index > 0) {
            index--;
            put_str(UART_CONSOLE, "\b \b");
         }
      } else if (index < BUFFER_SIZE - 1) {
         buffer[index++] = c;
         put_char(UART_CONSOLE, c);
      }
   }
   buffer[index] = '\0';
   put_char(UART_CONSOLE, '\n');
   return buffer;
}

void clear_screen(uint8_t uartPort) { put_str(uartPort, "\033[2J"); }

void hide_cursor(uint8_t uartPort) { put_str(uartPort, "\033[?25l"); }

void show_cursor(uint8_t uartPort) { put_str(uartPort, "\033[?25h"); }

void go_to_XY(uint8_t uartPort, uint8_t horizontalPosition, uint8_t verticalPosition) {
   char *aux = (char *)malloc(GOTOXY_BUFFER_SIZE);
   char *goToXYEscapeSequence = "\033[%d;%dH";
   snprintf(aux, BUFFER_SIZE, goToXYEscapeSequence, verticalPosition, horizontalPosition);
   put_str(uartPort, aux);
   free(aux);
}