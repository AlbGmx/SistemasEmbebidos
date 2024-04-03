#include "defs.h"

void app_main(void) {
   uint8_t playerState = PRE_LOAD, resultado, playAgain;
   char player;
   word_t testWord;
   char playerGuess;
   initUARTs();
   do {

      player = selectPlayer();

      putStr(UART_CONSOLE, "\n\rJugador seleccionado: ");
      putStr(UART_CONSOLE, (player == PLAYER_A) ? "A\n" : "B\n");

      if (player == PLAYER_A) {
         testWord = initCharState(testWord, getWordToGuess());
         playerState = PLAYING;
         clearBuffers();
         putStr(UART_CONSOLE, "\n\n\rEspere al Jugador B y presione una tecla cuando ambos estén listos: ");
         getChar(UART_CONSOLE);

         clearScreen();
         printGraphic(testWord);
         printWord(testWord);
         sendGameState(PLAYING);

         while (playerState == PLAYING) {
            clearBuffers();
            putStr(UART_CONSOLE, "\n\rEsperando al jugador B...");
            playerGuess = getChar(UART_ESP);
            putchar('\n');
            clearScreen();

            if (isValidGuess(playerGuess)) {
               printPlayerBGuess(playerGuess);
               resultado = checkGuess(&testWord, playerGuess);
               printResultToPlayerB(resultado);
               playerState = updatePlayersStates(resultado);

               printGraphic(testWord);
               printWord(testWord);
            }
         }

         clearScreen();

         testWord = setAllHits(testWord);
         printGraphic(testWord);
         printWord(testWord);

         if (playerState == GAME_WON) {
            putStr(UART_CONSOLE, "\n\rPerdió el Jugador B, Ganaste!\n");
            sendGameState(GAME_LOST);
         } else if (playerState == GAME_LOST) {
            putStr(UART_CONSOLE, "\n\rGanó el Jugador B, Perdiste!\n");
            sendGameState(GAME_WON);
         }

         putStr(UART_CONSOLE, "\n\rEsperando si Jugador B quiere jugar de nuevo...");
         playAgain = getChar(UART_ESP);
      } else {
         putStr(UART_CONSOLE, "\n\rEsperando al Jugador A... ");
         playerState = getValidStateFromPlayerA();
         clearScreen();
         while (playerState == PLAYING) {
            putStr(UART_CONSOLE, "\n\rJugador B, introduce un intento: ");
            playerState = sendGuessToPlayerA();
         }

         clearScreen();

         if (playerState == GAME_WON) {
            putStr(UART_CONSOLE, "\n\rPerdió el Jugador A, Ganaste!\n");
         } else
            putStr(UART_CONSOLE, "\n\rGanó el Jugador A, Perdiste!\n");

         putStr(UART_CONSOLE, "\n\r¿Desea jugar de nuevo? (S/N): ");
         playAgain = (toupper(getChar(UART_CONSOLE)) == 'S') ? 1 : 0;
         sendGameState(playAgain);
      }
   } while (playAgain);
   putStr(UART_CONSOLE, "\n\rGracias por jugar!");
}