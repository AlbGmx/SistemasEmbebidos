#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define TAG "ParkingMachine"
#define PARKING_PRICE 15
#define LED_DELAY_ON 500
#define LED_DELAY_OFF LED_DELAY_ON / 2
#define PRINTING_DELAY 2000
#define BUTTON_BOUNCE_TIME 150
#define RELEASED 0
#define PRESSED 1
#define MINIMUM_DELAY_MS 10

// LOGGING
#define LOG_DELAY 5       // Time Between Logs in Seconds
#define LOG_STATES false  // Set to false to disable state logging

// LEDS
#define LED_CHANGE_1_PESO GPIO_NUM_13
#define LED_CHANGE_5_PESOS GPIO_NUM_12
#define LED_CHANGE_10_PESOS GPIO_NUM_14
#define LED_PRINTING_RECEIPT GPIO_NUM_2
#define LED_NEEDLE_DOWN GPIO_NUM_27
#define LED_NEEDLE_MIDDLE GPIO_NUM_26
#define LED_NEEDLE_UP GPIO_NUM_25

// BUTTONS
#define BUTTON_1_PESO GPIO_NUM_18
#define BUTTON_5_PESOS GPIO_NUM_19
#define BUTTON_10_PESOS GPIO_NUM_21
#define BUTTON_20_PESOS GPIO_NUM_22
#define BUTTON_CAR_CROSSED GPIO_NUM_23
#define BUTTON_CAR_PRESENT GPIO_NUM_15

typedef enum {
   STATE_INITIAL = 0,
   STATE_WAITING_FOR_CAR,
   STATE_RECEIVING_MONEY,
   STATE_RETURNING_CHANGE,
   STATE_WAITING_CHANGE,
   STATE_RECEIPT,
   STATE_ELEVATING_NEEDLE,
   STATE_CAR_CROSSING,
   STATE_DESCENDING_NEEDLE
} states_t;

// Global variables
states_t currentState = STATE_INITIAL;
uint8_t change = 0;
uint8_t totalAmount = (uint8_t)-1;
int32_t lastStateChange = 0;
QueueHandle_t buttonQueueHandler;
uint32_t currentTime = 0;
bool areInterruptsAttached = false;
bool giveChange = false;
bool changeGiven = false;
bool isPressed = false;

void delayMillis(int millis) { vTaskDelay(millis / portTICK_PERIOD_MS); }

void configGPIOs() {
   gpio_config_t io_conf;

   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_OUTPUT;
   io_conf.pin_bit_mask = (1 << LED_CHANGE_1_PESO) | (1 << LED_CHANGE_5_PESOS) | (1 << LED_CHANGE_10_PESOS) |
                          (1 << LED_NEEDLE_DOWN) | (1 << LED_NEEDLE_MIDDLE) | (1 << LED_NEEDLE_UP) |
                          (1 << LED_PRINTING_RECEIPT);
   io_conf.pull_down_en = 0;
   io_conf.pull_up_en = 0;
   gpio_config(&io_conf);

   io_conf.intr_type = GPIO_INTR_NEGEDGE;
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pin_bit_mask = (1 << BUTTON_1_PESO) | (1 << BUTTON_5_PESOS) | (1 << BUTTON_10_PESOS) |
                          (1 << BUTTON_20_PESOS) | (1 << BUTTON_CAR_CROSSED) | (1 << BUTTON_CAR_PRESENT);
   io_conf.pull_down_en = 1;
   io_conf.pull_up_en = 0;
   gpio_config(&io_conf);
}

static void IRAM_ATTR buttonInterruptHandler(void *args) {
   uint32_t buttonActioned = (uint32_t)args;

   currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;

   if (currentTime - lastStateChange < BUTTON_BOUNCE_TIME) {
      return;
   }
   lastStateChange = currentTime;
   xQueueSendFromISR(buttonQueueHandler, &buttonActioned, NULL);
}

void parkingMachineTask(void *args) {
   int pinNumber;
   while (true) {
      if (xQueueReceive(buttonQueueHandler, &pinNumber, portMAX_DELAY)) {
         if (gpio_get_level(pinNumber) == RELEASED) {
            switch (currentState) {
               case STATE_WAITING_FOR_CAR:
                  if (pinNumber == BUTTON_CAR_PRESENT) {
                     currentState = STATE_RECEIVING_MONEY;
                     ESP_LOGW(TAG, "Automovil detectado, esperando dinero\n");
                  } else {
                     ESP_LOGE(TAG, "No se puede recibir dinero hasta que se detecte un automovil\n");
                  }
                  break;
               case STATE_RECEIVING_MONEY:
                  switch (pinNumber) {
                     case BUTTON_1_PESO:
                        totalAmount += 1;
                        ESP_LOGW(TAG, "Se han depositado 1 peso, total: %d\n", totalAmount);
                        break;
                     case BUTTON_5_PESOS:
                        totalAmount += 5;
                        ESP_LOGW(TAG, "Se han depositado 5 pesos, total: %d\n", totalAmount);
                        break;
                     case BUTTON_10_PESOS:
                        totalAmount += 10;
                        ESP_LOGW(TAG, "Se han depositado 10 pesos, total: %d\n", totalAmount);
                        break;
                     case BUTTON_20_PESOS:
                        totalAmount += 20;
                        ESP_LOGW(TAG, "Se han depositado 20 pesos, total: %d\n", totalAmount);
                        break;
                     default:
                        ESP_LOGE(TAG, "Operación no autorizada\n");
                        break;
                  }
                  if (totalAmount >= PARKING_PRICE) {
                     currentState = STATE_RETURNING_CHANGE;
                     ESP_LOGW(TAG, "Dinero suficiente, devolviendo %d de cambio\n", totalAmount - PARKING_PRICE);
                  }
                  break;
               case STATE_CAR_CROSSING:
                  if (pinNumber == BUTTON_CAR_CROSSED) {
                     currentState = STATE_DESCENDING_NEEDLE;
                     ESP_LOGW(TAG, "Automovil cruzó, bajando aguja\n");
                  }
                  break;
               default:
                  ESP_LOGE(TAG, "Operación no autorizada\n");
                  break;
            }
         }
      }
      delayMillis(MINIMUM_DELAY_MS);
   }
}

void configInterruptions() {
   buttonQueueHandler = xQueueCreate(10, sizeof(uint32_t));
   xTaskCreate(parkingMachineTask, "ParkingMachineTask", 2048, NULL, 1, NULL);

   gpio_install_isr_service(0);
   gpio_isr_handler_add(BUTTON_1_PESO, buttonInterruptHandler, (void *)BUTTON_1_PESO);
   gpio_isr_handler_add(BUTTON_5_PESOS, buttonInterruptHandler, (void *)BUTTON_5_PESOS);
   gpio_isr_handler_add(BUTTON_10_PESOS, buttonInterruptHandler, (void *)BUTTON_10_PESOS);
   gpio_isr_handler_add(BUTTON_20_PESOS, buttonInterruptHandler, (void *)BUTTON_20_PESOS);
   gpio_isr_handler_add(BUTTON_CAR_CROSSED, buttonInterruptHandler, (void *)BUTTON_CAR_CROSSED);
   gpio_isr_handler_add(BUTTON_CAR_PRESENT, buttonInterruptHandler, (void *)BUTTON_CAR_PRESENT);
}

void indicateReturnWithLedsTask(void *param) {
   while (true) {
      if (giveChange == true) {
         gpio_set_level(LED_CHANGE_1_PESO, 0);
         gpio_set_level(LED_CHANGE_5_PESOS, 0);
         gpio_set_level(LED_CHANGE_10_PESOS, 0);
         gpio_set_level(LED_PRINTING_RECEIPT, 0);
         while (change > 0) {
            if (change >= 10) {
               gpio_set_level(LED_CHANGE_10_PESOS, 1);
               delayMillis(LED_DELAY_ON * 2);
               gpio_set_level(LED_CHANGE_10_PESOS, 0);
               delayMillis(LED_DELAY_OFF * 2);
               change -= 10;
            } else if (change >= 5) {
               gpio_set_level(LED_CHANGE_5_PESOS, 1);
               delayMillis(LED_DELAY_ON * 2);
               gpio_set_level(LED_CHANGE_5_PESOS, 0);
               delayMillis(LED_DELAY_OFF * 2);
               change -= 5;
            } else if (change >= 1) {
               gpio_set_level(LED_CHANGE_1_PESO, 1);
               delayMillis(LED_DELAY_ON * 2);
               gpio_set_level(LED_CHANGE_1_PESO, 0);
               delayMillis(LED_DELAY_OFF * 2);
               change -= 1;
            }
         }
         ESP_LOGW(TAG, "Cambio devuelto\n");
         giveChange = false;
         changeGiven = true;
      }
      delayMillis(MINIMUM_DELAY_MS);
   }
}

void printReceivedReceipt() {
   gpio_set_level(LED_PRINTING_RECEIPT, 1);
   delayMillis(LED_DELAY_ON);
   gpio_set_level(LED_PRINTING_RECEIPT, 0);
   delayMillis(LED_DELAY_OFF);
   ESP_LOGW(TAG, "Recibo impreso, subiendo aguja\n");
}

void logCurrrentStateTask() {
   while (true) {
      switch (currentState) {
         case STATE_INITIAL:
            ESP_LOGI(TAG, "State %d -> Parking Machine Started\n", currentState);
            break;
         case STATE_WAITING_FOR_CAR:
            ESP_LOGI(TAG, "State %d -> Waiting for car\n", currentState);
            break;
         case STATE_RECEIVING_MONEY:
            ESP_LOGI(TAG, "State %d -> Receiving money\n", currentState);
            break;
         case STATE_RETURNING_CHANGE:
            ESP_LOGI(TAG, "State %d -> Returning change\n", currentState);
            break;
         case STATE_WAITING_CHANGE:
            ESP_LOGI(TAG, "State %d -> Waiting for change\n", currentState);
            break;
         case STATE_RECEIPT:
            ESP_LOGI(TAG, "State %d -> Printing receipt\n", currentState);
            break;
         case STATE_ELEVATING_NEEDLE:
            ESP_LOGI(TAG, "State %d -> Elevating needle\n", currentState);
            break;
         case STATE_CAR_CROSSING:
            ESP_LOGI(TAG, "State %d -> Car crossing\n", currentState);
            break;
         case STATE_DESCENDING_NEEDLE:
            ESP_LOGI(TAG, "State %d -> Descending needle\n", currentState);
            break;
         default:
            ESP_LOGI(TAG, "State %d -> Unknown State :/\n", currentState);
            break;
      }
      delayMillis(LOG_DELAY * 1000);
   }
}

void app_main() {
   configGPIOs();
   configInterruptions();
   xTaskCreate(indicateReturnWithLedsTask, "IndicateReturnWithLedsTask", 2048, NULL, 1, NULL);
   if (LOG_STATES) xTaskCreate(logCurrrentStateTask, "LogCurrrentStateTask", 2048, NULL, 1, NULL);

   while (true) {
      switch (currentState) {
         case STATE_INITIAL:
            if (totalAmount == (uint8_t)-1) {
               totalAmount = 0;
               gpio_set_level(LED_NEEDLE_DOWN, 1);
               ESP_LOGW(TAG, "Maquina inicializada\n");
            }
            currentState = STATE_WAITING_FOR_CAR;
            break;
         case STATE_RETURNING_CHANGE:
            change = totalAmount - PARKING_PRICE;
            giveChange = true;
            currentState = STATE_WAITING_CHANGE;
            break;
         case STATE_WAITING_CHANGE:
            if (changeGiven) {
               currentState = STATE_RECEIPT;
               changeGiven = false;
            }
            break;
         case STATE_RECEIPT:
            printReceivedReceipt();
            totalAmount = -1;
            currentState = STATE_ELEVATING_NEEDLE;
            break;
         case STATE_ELEVATING_NEEDLE:
            gpio_set_level(LED_NEEDLE_MIDDLE, 1);
            gpio_set_level(LED_NEEDLE_DOWN, 0);
            delayMillis(1000);
            gpio_set_level(LED_NEEDLE_UP, 1);
            gpio_set_level(LED_NEEDLE_MIDDLE, 0);
            currentState = STATE_CAR_CROSSING;
            break;
         case STATE_DESCENDING_NEEDLE:
            gpio_set_level(LED_NEEDLE_MIDDLE, 1);
            gpio_set_level(LED_NEEDLE_UP, 0);
            delayMillis(1000);
            gpio_set_level(LED_NEEDLE_DOWN, 1);
            gpio_set_level(LED_NEEDLE_MIDDLE, 0);
            currentState = STATE_INITIAL;
            break;
         default:
            break;
      }
      delayMillis(MINIMUM_DELAY_MS);
      currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
   }
}