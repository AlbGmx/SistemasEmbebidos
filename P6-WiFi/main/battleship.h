#ifndef battleship_h
#define battleship_h
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BOARD_SIZE 10
#define MAX_TURNS 30
#define TARGETS 20

int battleship();
void initialize_board();
void reset_game();
void process_shot(int x, int y);
void end_game();
void display_board();

#endif