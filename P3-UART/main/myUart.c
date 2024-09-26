#include "myUart.h"

static const char *TAG = "My Uart Library";
extern bool isReceiver;
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
   // Set UART_ESP
   ESP_ERROR_CHECK(uart_driver_install(UART_ESP, BUFFER_SIZE, 0, 0, NULL, 0));
   ESP_ERROR_CHECK(uart_param_config(UART_ESP, &uart_config));
   ESP_ERROR_CHECK(uart_set_pin(UART_ESP, UART_ESP_TX_PIN, UART_ESP_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
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
            if (!isReceiver) put_str(UART_CONSOLE, "\b \b");
         }
      } else if (index < BUFFER_SIZE - 1) {
         buffer[index++] = c;
         if (!isReceiver) put_char(UART_CONSOLE, c);
      }
   }
   buffer[index] = '\0';
   if (!isReceiver) put_char(UART_CONSOLE, '\n');
   return buffer;
}

void print_ascii_art(uint8_t uartPort, char *str) {
   clear_screen(uartPort);
   go_to_XY(uartPort, 0, 0);
   char *aux = (char *)malloc(BUFFER_SIZE);
   snprintf(aux, BUFFER_SIZE,
            "Minimum console size as set in program: %dx%d"
            "\nEquivalent to:\t%d letters per line and %d lines"
            "\nWaiting for data...\n",
            CONSOLE_WIDTH, CONSOLE_HEIGHT, DESIRED_LETTERS_PER_LINE, DESIRED_LINES);
   put_str(uartPort, aux);
   uint8_t x_pos = 0, y_pos = ASCII_FONT_HEIGHT;
   while (*str) {
      char nextChar = *str++;
      switch (get_char_type(nextChar)) {
         case ASCII_LETTER_TYPE:
            nextChar -= 'A';
            break;
         case ASCII_LETTER_LOWERCASE_TYPE:
            nextChar -= 'a';
            break;
         case ASCII_NUMBER_TYPE:
            nextChar = (nextChar - '0') + ASCII_NUMBERS_START_INDEX;
            break;
         case ASCII_SPACE_TYPE:
            nextChar = ASCII_SPACE_INDEX;
            break;
         case ASCII_EXCLAMATION_TYPE:
            nextChar = ASCII_EXCLAMATION_INDEX;
            break;
         case ASCII_DOT_TYPE:
            nextChar = ASCII_DOT_INDEX;
            break;
         case ASCII_PLUS_TYPE:
            nextChar = ASCII_PLUS_INDEX;
            break;
         case ASCII_MINUS_TYPE:
            nextChar = ASCII_MINUS_INDEX;
            break;
         default:
            continue;
      }

      print_single_ascii_art(x_pos, y_pos, (uint8_t)nextChar);
      x_pos += ASCII_FONT_WIDTH;
      if (x_pos > CONSOLE_WIDTH - ASCII_FONT_WIDTH) {
         x_pos = 0;
         y_pos += ASCII_FONT_HEIGHT + TWO_SPACES;
      }
   }
   put_char(uartPort, '\n');
}

ascii_type_t get_char_type(char c) {
   switch (c) {
      case EXCLAMATION:
         return ASCII_EXCLAMATION_TYPE;
         break;
      case DOT:
         return ASCII_DOT_TYPE;
         break;
      case PLUS:
         return ASCII_PLUS_TYPE;
         break;
      case MINUS:
         return ASCII_MINUS_TYPE;
         break;
      case SPACE:
         return ASCII_SPACE_TYPE;
         break;
      case NEWLINE:
         return ASCII_NEWLINE_TYPE;
         break;
      case CARRIAGE_RETURN:
         return ASCII_CARRIAGE_RETURN_TYPE;
         break;
      default:
         if (c >= START_UPPERCASE && c <= END_UPPERCASE) return ASCII_LETTER_TYPE;
         if (c >= START_LOWERCASE && c <= END_LOWERCASE) return ASCII_LETTER_LOWERCASE_TYPE;
         if (c >= ASCII_NUMBERS_START && c <= ASCII_NUMBERS_END) return ASCII_NUMBER_TYPE;
   }
   return ASCII_ANOTHER_TYPE;
}

void print_single_ascii_art(uint8_t x_pos, uint8_t y_pos, uint8_t nextChar) {
   go_to_XY(UART_CONSOLE, x_pos, y_pos);
   put_str(UART_CONSOLE, (char *)asciiFont[nextChar]);
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