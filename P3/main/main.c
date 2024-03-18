/* UART Echo Example

	This example code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <string.h>
#include "myUart.h"

#define BUF_SIZE (1024)
#define UART_RX_PIN (3)
#define UART_TX_PIN (1)

#define UART_RX_PIN_2 (16)
#define UART_TX_PIN_2 (17)

static const char *TAG = "UART TEST";

void app_main(void) {
	initUARTs();
	ESP_LOGI(TAG, "UART Echo Example...");
	int aux = UART_kbHit();
	ESP_LOGI(TAG, "kbHit: %d", aux);
}
