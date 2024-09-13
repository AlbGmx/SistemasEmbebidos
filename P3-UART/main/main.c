#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_err.h"
#include <stdlib.h>
#include "driver/gpio.h"

#define UART0_TXD (GPIO_NUM_1)   // TX pin for UART0
#define UART0_RXD (GPIO_NUM_3)   // RX pin for UART0
#define UART2_TXD (GPIO_NUM_17)  // TX pin for UART2
#define UART2_RXD (GPIO_NUM_16)  // RX pin for UART2

#define ECHO_UART0_PORT_NUM (UART_NUM_0)
#define ECHO_UART2_PORT_NUM (UART_NUM_2)
#define ECHO_UART_BAUD_RATE  (115200)
#define ECHO_TASK_STACK_SIZE (2048)  // Increased stack size

static const char *TAG = "UART TEST";

#define BUF_SIZE (1024)

static void uart2_to_uart0_task(void *arg)
{
    uart_config_t uart0_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_config_t uart2_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    int intr_alloc_flags = ESP_INTR_FLAG_IRAM;

    // Install UART drivers
    esp_err_t ret;
    ret = uart_driver_install(ECHO_UART0_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART0 driver install failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);  // Exit the task
    }

    ret = uart_driver_install(ECHO_UART2_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART2 driver install failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);  // Exit the task
    }

    // Configure UART parameters
    ret = uart_param_config(ECHO_UART0_PORT_NUM, &uart0_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART0 param config failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);  // Exit the task
    }

    ret = uart_param_config(ECHO_UART2_PORT_NUM, &uart2_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART2 param config failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);  // Exit the task
    }

    // Set UART pins
    ret = uart_set_pin(ECHO_UART0_PORT_NUM, UART0_TXD, UART0_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART0 set pin failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);  // Exit the task
    }

    ret = uart_set_pin(ECHO_UART2_PORT_NUM, UART2_TXD, UART2_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART2 set pin failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);  // Exit the task
    }

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for data buffer");
        vTaskDelete(NULL);  // Exit the task
    }

    while (1) {
        // Read from UART2
        int len = uart_read_bytes(ECHO_UART0_PORT_NUM, data, BUF_SIZE - 1, 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            data[len] = '\0';  // Ensure null-termination
            // Send data to UART0
            ESP_LOGI(TAG, "Read %d bytes from UART2 to UART0", len);
            uart_write_bytes(ECHO_UART2_PORT_NUM, (const char *) data, len);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    free(data);  // Ensure allocated memory is freed
    vTaskDelete(NULL);  // Exit the task
}

void app_main(void)
{
    xTaskCreate(uart2_to_uart0_task, "uart2_to_uart0_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
