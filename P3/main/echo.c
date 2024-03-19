/*
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
#define UART_RX_PIN_2 (16)
#define UART_TX_PIN_2 (17)
#define MY_UART_RX 2
#define MY_UART_TX 4


static const char *TAG_ESP0 = "UART ESP0";
static const char *TAG_ESP1 = "UART ESP1";
static const char *TAG_ESP2 = "UART ESP2";

static void echoTaskESP2(void *arg) {
	// Configure a temporary buffer for the incoming data
	uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

	while (1) {
		// Read data from the UART
		int len = uart_read_bytes(UART_NUM_2, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG_ESP2, "\n");
		// Write data back to the UART
		uart_write_bytes(UART_NUM_0, (const char *)data, len);
	}
}

static void echoTaskESP1(void *arg) {
	// Configure a temporary buffer for the incoming data
	uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

	while (1) {
		// Read data from the UART
		int len = uart_read_bytes(UART_NUM_2, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG_ESP1, "\n");
		// Write data back to the UART
		uart_write_bytes(UART_NUM_0, (const char *)data, len);
	}
}
static void echoTaskESP0(void *arg) {
	// Configure a temporary buffer for the incoming data
	uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

	while (1) {
		// Read data from the UART
		int len = uart_read_bytes(UART_NUM_0, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG_ESP0, "\n");
		// Write data back to the UART
		uart_write_bytes(UART_NUM_2, (const char *)data, len);
	}
}

void app_main(void) {

	uart_config_t uart_config = {
		 .baud_rate	 = 115200,
		 .data_bits	 = UART_DATA_8_BITS,
		 .parity		 = UART_PARITY_DISABLE,
		 .stop_bits	 = UART_STOP_BITS_1,
		 .flow_ctrl	 = UART_HW_FLOWCTRL_DISABLE,
		 .source_clk = UART_SCLK_DEFAULT,
	};
	// Configure UART parameters
	ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
	ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
	ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));

	// Set UART pins
	ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
	ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, MY_UART_TX, MY_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
	ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, UART_TX_PIN_2, UART_RX_PIN_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

	// Setup UART buffered IO with event queue
	QueueHandle_t uart_queue;
	// Install UART driver using an event queue here
	ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0));
	ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0));
	ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0));
	xTaskCreate(echoTaskESP0, "echoTaskESP0", CONFIG_ESP_MAIN_TASK_STACK_SIZE, NULL, 10, NULL);
	xTaskCreate(echoTaskESP2, "echoTaskESP2", CONFIG_ESP_MAIN_TASK_STACK_SIZE, NULL, 10, NULL);
}
*/