#ifndef PONG_H
#define PONG_H

#include "console.h"
#include "driver/gpio.h"

// ASCII Chars
#define FLOOR '-'
#define CEILING FLOOR
#define WALL '|'
#define TOP_LEFT '/'
#define TOP_RIGHT '\\'
#define BOTTOM_LEFT TOP_RIGHT
#define BOTTOM_RIGHT TOP_LEFT
#define BALL 'o'
#define PADDLE '='
#define BORDER_WIDTH 1
#define CONSOLE_HEIGHT 25
#define CONSOLE_WIDTH 40
#define PADDLE_SIZE 5
#define PADDLE_SYMBOL PADDLE
#define BALL_SYMBOL BALL
#define BALL_SPEED 1
#define PADDLE_SPEED 3
#define BALL_INITIAL_X CONSOLE_WIDTH / 2
#define BALL_INITIAL_Y CONSOLE_HEIGHT / 2
#define PADDLE_FIXED_Y CONSOLE_HEIGHT - PADDLE_SIZE
#define PADDLE_INITIAL_X CONSOLE_WIDTH / 2 - PADDLE_SIZE / 2
#define MINIMUM_DELAY_MS 1
#define BUTTON_BOUNCE_TIME 20
#define RELEASED 1
#define BALL_UPDATE_DELAY 1000
#define BALL_STEP_SIZE 1
#define BALL_STOPPED 0
#define MOVING_RIGHT true
#define MOVING_LEFT false
#define MOVING_DOWN true
#define MOVING_UP false
// Sync event group
#define GAME_STARTED (1 << 0)
#define GAME_ENDED (1 << 1)
#define GAME_LOST (1 << 2)
#define ALL_EVENTS (GAME_STARTED | GAME_ENDED | GAME_LOST)
// Buttons
#define BUTTON_LEFT GPIO_NUM_18
#define BUTTON_RIGHT GPIO_NUM_22
#define BUTTON_END GPIO_NUM_23
// LEDs
#define LED_DEBUG GPIO_NUM_2
#define LED_1S GPIO_NUM_13
#define LED_2S GPIO_NUM_12
#define LED_4S GPIO_NUM_14
#define LED_8S GPIO_NUM_27
#define LED_16S GPIO_NUM_26
#define LED_32S GPIO_NUM_25

typedef struct {
   uint8_t x;
   uint8_t y;
   uint8_t last_x;
   uint8_t size;
   char speed;
   char symbol;
} paddle_t;

typedef struct {
   uint8_t x;
   uint8_t y;
   int8_t last_x;
   int8_t last_y;
   bool vertical_direction;
   bool horizontal_direction;
   char symbol;
} ball_t;

typedef struct {
   paddle_t paddle;
   ball_t ball;
} game_elements_t;

void endTask(bool*);
void displayPointsTask(uint64_t*);
void deletePaddle(paddle_t*);
void printPaddle(paddle_t*);
void updatePaddle(paddle_t*);
void deleteBall(ball_t*);
void printBall(ball_t*);
void updateBall(ball_t*);
void calculateBallCollisionsTask(game_elements_t*);
void renderGameTask(game_elements_t*);
void printOutline();

#endif