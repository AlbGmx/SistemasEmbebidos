#include "console.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "pong.h"

#define TAG "Tasks"
uint64_t points = 0;
QueueHandle_t buttonQueueHandler;
EventGroupHandle_t xGameEventSync;
int32_t lastStateChange = 0;
uint32_t currentTime = 0;
game_elements_t gameElements = {
    .paddle = {.y = PADDLE_FIXED_Y,
               .x = PADDLE_INITIAL_X,
               .last_x = 1,
               .size = PADDLE_SIZE,
               .speed = PADDLE_SPEED,
               .symbol = PADDLE_SYMBOL},
    .ball = {.x = BALL_INITIAL_X,
             .y = BALL_INITIAL_Y,
             .last_x = 1,
             .last_y = 1,
             .symbol = BALL_SYMBOL,
             .vertical_direction = MOVING_UP,
             .horizontal_direction = MOVING_LEFT},
};

void delayMillis(int millis) { vTaskDelay(pdMS_TO_TICKS(millis)); }

void configGPIOs() {
   gpio_config_t io_conf;

   io_conf.intr_type = GPIO_INTR_NEGEDGE;
   io_conf.mode = GPIO_MODE_INPUT;
   io_conf.pin_bit_mask = (1 << BUTTON_LEFT) | (1 << BUTTON_RIGHT) | (1 << BUTTON_END);
   io_conf.pull_down_en = 1;
   io_conf.pull_up_en = 0;
   gpio_config(&io_conf);

   io_conf.intr_type = GPIO_INTR_DISABLE;
   io_conf.mode = GPIO_MODE_OUTPUT;
   io_conf.pin_bit_mask =
       (1 << LED_DEBUG) | (1 << LED_1S) | (1 << LED_2S) | (1 << LED_4S) | (1 << LED_8S) | (1 << LED_16S);
   io_conf.pull_down_en = 0;
   io_conf.pull_up_en = 0;
   gpio_config(&io_conf);
}

static void IRAM_ATTR buttonInterruptHandler(void *args) {
   uint32_t buttonActioned = (uint32_t)args;
   currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;
   if (currentTime - lastStateChange < BUTTON_BOUNCE_TIME) {
      return;
   }
   if (buttonActioned == BUTTON_LEFT) {
      gpio_set_level(LED_DEBUG, 1);
   } else if (buttonActioned == BUTTON_RIGHT) {
      gpio_set_level(LED_DEBUG, 0);
   }
   lastStateChange = currentTime;
   xQueueSendFromISR(buttonQueueHandler, &buttonActioned, NULL);
}

void movePaddle(void *args) {
   int pinNumber;
   while (true) {
      if (xQueueReceive(buttonQueueHandler, &pinNumber, portMAX_DELAY)) {
         if (pinNumber == BUTTON_LEFT) {
            if (gameElements.paddle.x > 0 + BORDER_WIDTH + gameElements.paddle.speed) {
               gameElements.paddle.x -= gameElements.paddle.speed;
            } else {
               gameElements.paddle.x = 0 + BORDER_WIDTH;
            }
         } else if (pinNumber == BUTTON_RIGHT) {
            if (gameElements.paddle.x <
                CONSOLE_WIDTH - gameElements.paddle.size - BORDER_WIDTH - gameElements.paddle.speed) {
               gameElements.paddle.x += gameElements.paddle.speed;
            } else {
               gameElements.paddle.x = CONSOLE_WIDTH - gameElements.paddle.size;
            }
         } else
            xEventGroupSetBits(xGameEventSync, GAME_ENDED);
      }
      delayMillis(MINIMUM_DELAY_MS);
   }
}

void configInterruptions() {
   buttonQueueHandler = xQueueCreate(10, sizeof(uint32_t));
   xTaskCreate(movePaddle, "Move Paddle Task", 2048, NULL, 1, NULL);

   gpio_install_isr_service(0);
   gpio_isr_handler_add(BUTTON_LEFT, buttonInterruptHandler, (void *)BUTTON_LEFT);
   gpio_isr_handler_add(BUTTON_RIGHT, buttonInterruptHandler, (void *)BUTTON_RIGHT);
   gpio_isr_handler_add(BUTTON_END, buttonInterruptHandler, (void *)BUTTON_END);
}

void startGame() {
   xTaskCreatePinnedToCore((TaskFunction_t)renderGameTask, "Render the Game", 2048, &gameElements, 1, NULL, 0);
   xTaskCreate((TaskFunction_t)displayPointsTask, "Show Score", 2048, &points, 1, NULL);
}

void app_main() {
   configGPIOs();
   initializeUart();
   configInterruptions();
   gameElements.ball.y = esp_random() % (CONSOLE_HEIGHT / 2) + 1;
   gameElements.ball.x = esp_random() % CONSOLE_WIDTH;
   startGame();
   xGameEventSync = xEventGroupCreate();
   xEventGroupWaitBits(xGameEventSync, GAME_ENDED, pdTRUE, pdTRUE, portMAX_DELAY);
}