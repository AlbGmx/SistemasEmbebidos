#include "battleship.h"

// Game board (0 = empty, 1 = ship part, -1 = hit ship part)
int board[BOARD_SIZE][BOARD_SIZE] = {0};
int turns_left = MAX_TURNS;
int score = 0;
int game_over = 0;  // 0 = ongoing, 1 = won, -1 = lost

int battleship() {
   srand(time(0));  // Seed for random ship placement
   reset_game();    // Initialize the game state

   int x, y;
   while (game_over == 0 && turns_left > 0) {
      display_board();
      printf("Enter coordinates (x y) to fire: ");
      scanf("%d %d", &x, &y);

      // Check if input is within bounds
      if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
         printf("Invalid coordinates. Try again.\n");
         continue;
      }

      process_shot(x, y);
   }
   end_game();  // Display end of game message
   return 0;
}

// Initialize game board with randomly placed ships
void initialize_board() {
   for (int i = 0; i < TARGETS; i++) {
      int x = rand() % BOARD_SIZE;
      int y = rand() % BOARD_SIZE;
      if (board[x][y] == 0) {  // Only place on empty cell
         board[x][y] = 1;
      } else {
         i--;  // Retry if the cell is already occupied
      }
   }
}

// Reset the game variables and board
void reset_game() {
   turns_left = MAX_TURNS;
   score = 0;
   game_over = 0;
   // Clear the board
   for (int i = 0; i < BOARD_SIZE; i++) {
      for (int j = 0; j < BOARD_SIZE; j++) {
         board[i][j] = 0;
      }
   }
   initialize_board();
}

// Process a shot at given coordinates
void process_shot(int x, int y) {
   if (game_over != 0) return;

   if (board[x][y] == 1) {  // Hit
      board[x][y] = -1;     // Mark as hit
      score++;
      turns_left++;  // Extra turn for hit
      printf("Hit! Score: %d, Turns left: %d\n", score, turns_left);
   } else {  // Miss
      turns_left--;
      printf("Miss. Turns left: %d\n", turns_left);
   }

   // Check for game end conditions
   if (score == TARGETS) {
      game_over = 1;
   } else if (turns_left == 0) {
      game_over = -1;
   }
}

// Display end of game message
void end_game() {
   display_board();  // Final board state
   if (game_over == 1) {
      printf("Congratulations, you won!\n");
   } else if (game_over == -1) {
      printf("Game over! You've run out of turns.\n");
   }
}

// Display the current state of the board
void display_board() {
   printf("\nBoard:\n");
   for (int i = 0; i < BOARD_SIZE; i++) {
      for (int j = 0; j < BOARD_SIZE; j++) {
         if (board[i][j] == -1) {
            printf(" X ");  // Hit
         } else if (board[i][j] == 1) {
            printf(" ~ ");  // Undiscovered ship part
         } else {
            printf(" . ");  // Empty water
         }
      }
      printf("\n");
   }
   printf("\n");
}
