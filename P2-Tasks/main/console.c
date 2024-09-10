#include "console.h"

#define TAG "Console"

void clearScreen() { printf("\033[2J"); }

void hideCursor() { printf("\033[?25l"); }

void showCursor() { printf("\033[?25h"); }

void initializeUart() {
   uart_config_t uart_config = {
       .baud_rate = 115200,
       .data_bits = UART_DATA_8_BITS,
       .parity = UART_PARITY_DISABLE,
       .stop_bits = UART_STOP_BITS_2,
       .flow_ctrl = UART_HW_FLOWCTRL_RTS,
       .rx_flow_ctrl_thresh = 122,
   };
   ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
}

void goToXY(int horizontalPosition, int verticalPosition) {
   printf("\033[%d;%dH", verticalPosition + 1, horizontalPosition + 1);
}