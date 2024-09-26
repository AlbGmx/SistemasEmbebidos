
#include "myUart.h"

#define IS_RECEIVER_CONTROL_PIN GPIO_NUM_23
#define SHOW_MODE_PIN GPIO_NUM_2

static const char *TAG = "P3 - UART";
bool isReceiver = false;

void gpio_setup() {
   gpio_config_t io_conf;
   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
   io_conf.pin_bit_mask = 1ULL << IS_RECEIVER_CONTROL_PIN | 1ULL << SHOW_MODE_PIN;
   io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
   io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
   gpio_config(&io_conf);
   ESP_LOGI(TAG, "GPIO setup complete");
}

void app_main(void) {
   init_UARTs();
   gpio_setup();
   char *data = (char *)malloc(BUFFER_SIZE);

   // Set mode
   uint8_t uartReceiving = (gpio_get_level(IS_RECEIVER_CONTROL_PIN) == 0) ? UART_NUM_0 : UART_NUM_2;
   uint8_t uartTransmitting = (uartReceiving == UART_NUM_0) ? UART_NUM_2 : UART_NUM_0;
   if (uartReceiving == UART_NUM_2) isReceiver = true;
   gpio_set_level(SHOW_MODE_PIN, isReceiver);
   ESP_LOGI(TAG, "Receiving on UART%d, Transmitting on UART%d", uartReceiving, uartTransmitting);

   while (true) {
      if (isReceiver) {
         int len = uart_read_bytes(uartReceiving, (uint8_t *)data, BUFFER_SIZE, 10 / portTICK_PERIOD_MS);
         if (len > 0) {
            data[len] = '\0';
            print_ascii_art(uartTransmitting, data);
         }
      } else {
         put_str(uartReceiving, "Enter data: ");
         data = get_line(uartReceiving);
         put_str(uartTransmitting, data);
      }
   }
   free(data);
}
