#include "defs.h"

word_t initCharState(word_t word, char *guessWord) {
   uint8_t i;
   word.guessWord = guessWord;
   word.len = strlen(guessWord);
   for (i = 0; i < word.len; i++) {
      word.hits[i] = HIDDEN_CHAR;
   }
   word.misses = 0;
   word.guesses = 0;
   return word;
}

uint8_t selectPlayer() {
   char *c;
   clearScreen();

   printf("\n\nSelecciona el jugador (A o B): ");
   while (1) {
      fflush(stdout);
      c = getLine(UART_CONSOLE);
      if (toupper(*c) == 'A') return PLAYER_A;
      if (toupper(*c) == 'B') return PLAYER_B;

      printf("\nJugador invalido, intenta de nuevo: ");
   }
}

void printGraphic(word_t word) {
   printf("\n\n");
   printf("\t\t  +---+\n");
   printf("\t\t  |   |\n");
   printf("\t\t  %c   |\n", (word.misses > 0) ? 'O' : ' ');
   printf("\t\t %c%c%c  |\n", (word.misses > 2) ? '/' : ' ', (word.misses > 1) ? '|' : ' ',
          (word.misses > 3) ? '\\' : ' ');
   printf("\t\t %c %c  |\n", (word.misses > 4) ? '/' : ' ', (word.misses > 5) ? '\\' : ' ');
   printf("\t\t      |\n");
   printf("\t\t=========\n\n");
}

void printWord(word_t word) {
   uint8_t i;
   printf("\n\t");
   for (i = 0; i < word.len; i++) {
      printf("%c ", (word.hits[i] == FOUND_CHAR) ? word.guessWord[i] : '_');
   }
   printf("\n\n");
}

uint8_t isWordGuessed(word_t word) {
   uint8_t i;
   for (i = 0; i < word.len; i++) {
      if (word.hits[i] == HIDDEN_CHAR) return WORD_NOT_GUESSED;
   }
   return WORD_GUESSED;
}

uint8_t checkGuess(word_t *word, char guess) {
   uint8_t i;
   uint8_t found = 0;

   for (i = 0; i < word->guesses + word->misses; i++) {
      if (toupper(word->lettersTried[i]) == toupper(guess)) {
         word->misses++;
         if (word->misses >= MAX_MISSES) return NO_TRIES_LEFT;
         return LETTER_ALREADY_GUESSED;
      }
   }

   for (i = 0; i < word->len; i++) {
      if (toupper(word->guessWord[i]) == toupper(guess)) {
         if (word->hits[i] == FOUND_CHAR) {
            word->misses++;
            return LETTER_ALREADY_GUESSED;
         }
         word->hits[i] = FOUND_CHAR;
         found = 1;
      }
   }
   if (found) {
      word->lettersTried[word->misses + word->guesses] = toupper(guess);
      if (isWordGuessed(*word)) return WORD_GUESSED;
      word->guesses++;
      return LETTER_FOUND;
   } else {
      word->misses++;
   }
   return word->misses >= MAX_MISSES ? NO_TRIES_LEFT : NO_CHANGE;
}

word_t setAllHits(word_t word) {
   uint8_t i;
   for (i = 0; i < word.len; i++) {
      word.hits[i] = FOUND_CHAR;
   }
   return word;
}

void sendGameState(uint8_t state) { putChar(UART_ESP, state); }

void printPlayerBGuess(char playerGuess) {
   if (playerGuess) {
      putStr(UART_CONSOLE, "\n\nJugador B adivinó la letra: \'");
      putChar(UART_CONSOLE, playerGuess);
      putStr(UART_CONSOLE, "\'\n");
   }
}

uint8_t isValidGuess(char playerGuess) { return isalpha(playerGuess); }

char sendGuessToPlayerA() {
   char *playerB_Guess;
   clearBuffer(UART_ESP);
   playerB_Guess = getLine(UART_CONSOLE);

   while (!isValidGuess(playerB_Guess[0])) {
      putStr(UART_CONSOLE, "\n\rLetra invalida, intenta de nuevo (a-z, A-Z): ");
      playerB_Guess = getLine(UART_CONSOLE);
   }
   putStr(UART_ESP, playerB_Guess);

   return getValidStateFromPlayerA();
}

char getValidStateFromPlayerA() {
   clearBuffer(UART_ESP);
   char gameState = getChar(UART_ESP);
   return gameState;
}
void clearBuffer(int uartPort) {
   uint8_t data;
   while (uart_read_bytes(uartPort, &data, ONE_BYTE, 0) != 0) {
   }
}

void clearBuffers() {
   clearBuffer(UART_ESP);
   clearBuffer(UART_CONSOLE);
}

void printResultToPlayerB(uint8_t resultado) {
   switch (resultado) {
      case NO_CHANGE:
         putStr(UART_CONSOLE, "\n\rLetra no encontrada!\n");
         break;
      case WORD_GUESSED:
         putStr(UART_CONSOLE, "\n\rPalabra adivinada!\n");
         break;
      case LETTER_FOUND:
         putStr(UART_CONSOLE, "\n\rLetra encontrada!\n");
         break;
      case NO_TRIES_LEFT:
         putStr(UART_CONSOLE, "\n\rYa no quedan intentos\n Perdiste!\n");
         break;
      case LETTER_ALREADY_GUESSED:
         putStr(UART_CONSOLE, "\n\rLetra ya adivinada!\n");
      default:
         break;
   }
}

uint8_t updatePlayersStates(uint8_t resultado) {
   switch (resultado) {
      case WORD_GUESSED:
         sendGameState(GAME_WON);
         return GAME_LOST;
         break;
      case NO_TRIES_LEFT:
         sendGameState(GAME_LOST);
         return GAME_WON;
         break;
      case LETTER_FOUND:
      case NO_CHANGE:
      case LETTER_ALREADY_GUESSED:
      default:
         sendGameState(PLAYING);
         return PLAYING;
   }
}

char *getWordToGuess() {
   putStr(UART_CONSOLE, "\n\rIntroduce la palabra a adivinar: ");
   return getLine(UART_CONSOLE);
}
