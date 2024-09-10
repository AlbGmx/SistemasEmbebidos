#include "pong.h"

#include "console.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "math.h"
#define TAG "Pong"

extern EventGroupHandle_t xGameEventSync;
extern uint64_t points;

void renderGameTask(game_elements_t* elements) {
   bool inGame = true;
   hideCursor();
   clearScreen();
   printOutline();
   xTaskCreatePinnedToCore((TaskFunction_t)calculateBallCollisionsTask, "BallPositionTask", 2048, elements, 1, NULL,
                           xPortGetCoreID());
   xTaskCreatePinnedToCore((TaskFunction_t)endTask, "End Task", 2048, &inGame, 1, NULL, xPortGetCoreID());
   while (inGame) {
      goToXY((CONSOLE_WIDTH / 2) - 6, 0);
      printf("---\n");
      updatePaddle(&(elements->paddle));
      updateBall(&(elements->ball));
      delayMillis(1);
   }
   vTaskDelete(NULL);
}

void displayPointsTask(uint64_t* points) {
   while (true) {
      // display the points USING 5 LEDs in binary
      gpio_set_level(LED_1S, *points & 0x01);
      gpio_set_level(LED_2S, *points & 0x02);
      gpio_set_level(LED_4S, *points & 0x04);
      gpio_set_level(LED_8S, *points & 0x08);
      gpio_set_level(LED_16S, *points & 0x10);
      delayMillis(100);
   }
}

void printOutline() {
   for (int horizontalIndex = 0; horizontalIndex <= CONSOLE_WIDTH; horizontalIndex++) {
      for (int verticalIndex = 0; verticalIndex <= CONSOLE_HEIGHT; verticalIndex++) {
         goToXY(horizontalIndex, verticalIndex);
         switch (horizontalIndex) {
            case 0:
               switch (verticalIndex) {
                  case 0:
                     printf("%c", TOP_LEFT);
                     break;
                  case CONSOLE_HEIGHT:
                     printf("%c", BOTTOM_LEFT);
                     break;
                  default:
                     printf("%c", WALL);
                     break;
               }
               break;
            case CONSOLE_WIDTH:
               switch (verticalIndex) {
                  case 0:
                     printf("%c", TOP_RIGHT);
                     break;
                  case CONSOLE_HEIGHT:
                     printf("%c", BOTTOM_RIGHT);
                     break;
                  default:
                     printf("%c", WALL);
                     break;
               }
               break;
            default:
               switch (verticalIndex) {
                  case 0:
                  case CONSOLE_HEIGHT:
                     printf("%c", FLOOR);
                     break;
                  default:
                     break;
               }
         }
      }
   }
}

void deleteBall(ball_t* ball) {
   goToXY(ball->last_x, ball->last_y);
   printf(" ");
}

void calculateBallCollisionsTask(game_elements_t* gameElements) {
   while (true) {
      goToXY(CONSOLE_WIDTH + 1, 1);
      // Ball Going Right
      if (gameElements->ball.horizontal_direction == MOVING_RIGHT) {
         if (gameElements->ball.x > CONSOLE_WIDTH - BORDER_WIDTH - BALL_STEP_SIZE) {
            // Bounce
            gameElements->ball.horizontal_direction = MOVING_LEFT;
         }
      }
      // Ball Going Left
      else if (gameElements->ball.horizontal_direction == MOVING_LEFT) {
         // Enough space to move left
         if (gameElements->ball.x < BALL_STEP_SIZE + BORDER_WIDTH)
            // Bounce
            gameElements->ball.horizontal_direction = MOVING_RIGHT;
      }

      // Ball Going Down
      if (gameElements->ball.vertical_direction == MOVING_DOWN) {
         if (gameElements->ball.y >= CONSOLE_HEIGHT) {
            // End Game
            xEventGroupSetBits(xGameEventSync, GAME_LOST);
            vTaskDelete(NULL);
            gameElements->ball.vertical_direction = MOVING_UP;
            // Paddle Collision
         } else if (gameElements->ball.y == gameElements->paddle.y && gameElements->ball.x >= gameElements->paddle.x &&
                    gameElements->ball.x <= gameElements->paddle.x + gameElements->paddle.size) {
            points++;
            gameElements->ball.vertical_direction = MOVING_UP;
         }
      }
      // Ball Going Up
      else if (gameElements->ball.vertical_direction == MOVING_UP) {
         if (gameElements->ball.y <= BORDER_WIDTH + gameElements->ball.vertical_direction) {
            // Bounce
            gameElements->ball.vertical_direction = MOVING_DOWN;
         }
      }

      gameElements->ball.x += BALL_STEP_SIZE * (gameElements->ball.horizontal_direction == MOVING_RIGHT ? 1 : -1);
      gameElements->ball.y += BALL_STEP_SIZE * (gameElements->ball.vertical_direction == MOVING_DOWN ? 1 : -1);

      uint64_t delay = BALL_UPDATE_DELAY / ((points + 1) < 1 ? 1 : points + 1);
      delayMillis((delay < MINIMUM_DELAY_MS) ? MINIMUM_DELAY_MS : delay);
   }
}

void endTask(bool* endGame) {
   xEventGroupWaitBits(xGameEventSync, (GAME_LOST), pdTRUE, pdTRUE, portMAX_DELAY);
   *endGame = false;
   goToXY(CONSOLE_WIDTH / 2 - 12, CONSOLE_HEIGHT / 2);
   printf("Game Over: Points %3lld\n", points);
   vTaskDelete(NULL);
}

void printBall(ball_t* ball) {
   goToXY(ball->x, ball->y);
   printf("%c", ball->symbol);
}

void updateBall(ball_t* ball) {
   deleteBall(ball);
   printBall(ball);
   ball->last_x = ball->x;
   ball->last_y = ball->y;
}

void deletePaddle(paddle_t* paddle) {
   goToXY(paddle->last_x, paddle->y);
   for (int i = 0; i <= paddle->size; i++) {
      printf(" ");
   }
}

void printPaddle(paddle_t* paddle) {
   goToXY(paddle->x, paddle->y);
   for (int i = 0; i < paddle->size; i++) {
      printf("%c", paddle->symbol);
   }
}

void updatePaddle(paddle_t* paddle) {
   if (paddle->x == paddle->last_x) {
      return;
   }
   deletePaddle(paddle);
   printPaddle(paddle);
   paddle->last_x = paddle->x;
}
