#include "asciiArt.h"
const char *asciiFont[] = {
    // Letras A-Z (índices 0-25)
    "  ###  \e[B\x1b[7D #   # \e[B\x1b[7D ##### \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D",  // A
    " ####  \e[B\x1b[7D #   # \e[B\x1b[7D ####  \e[B\x1b[7D #   # \e[B\x1b[7D ####  \e[B\x1b[7D",  // B
    "  #### \e[B\x1b[7D #     \e[B\x1b[7D #     \e[B\x1b[7D #     \e[B\x1b[7D  #### \e[B\x1b[7D",  // C
    " ####  \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D ####  \e[B\x1b[7D",  // D
    " ##### \e[B\x1b[7D #     \e[B\x1b[7D ##### \e[B\x1b[7D #     \e[B\x1b[7D ##### \e[B\x1b[7D",  // E
    " ##### \e[B\x1b[7D #     \e[B\x1b[7D ##### \e[B\x1b[7D #     \e[B\x1b[7D #     \e[B\x1b[7D",  // F
    "  #### \e[B\x1b[7D #     \e[B\x1b[7D #  ## \e[B\x1b[7D #   # \e[B\x1b[7D  #### \e[B\x1b[7D",  // G
    " #   # \e[B\x1b[7D #   # \e[B\x1b[7D ##### \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D",  // H
    "  ###  \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D  ###  \e[B\x1b[7D",  // I
    "   ### \e[B\x1b[7D    #  \e[B\x1b[7D    #  \e[B\x1b[7D #  #  \e[B\x1b[7D  ##   \e[B\x1b[7D",  // J
    " #   # \e[B\x1b[7D #  #  \e[B\x1b[7D ###   \e[B\x1b[7D #  #  \e[B\x1b[7D #   # \e[B\x1b[7D",  // K
    " #     \e[B\x1b[7D #     \e[B\x1b[7D #     \e[B\x1b[7D #     \e[B\x1b[7D ##### \e[B\x1b[7D",  // L
    " #   # \e[B\x1b[7D ## ## \e[B\x1b[7D # # # \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D",  // M
    " #   # \e[B\x1b[7D ##  # \e[B\x1b[7D # # # \e[B\x1b[7D #  ## \e[B\x1b[7D #   # \e[B\x1b[7D",  // N
    "  ###  \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D  ###  \e[B\x1b[7D",  // O
    " ####  \e[B\x1b[7D #   # \e[B\x1b[7D ####  \e[B\x1b[7D #     \e[B\x1b[7D #     \e[B\x1b[7D",  // P
    "  ###  \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D #  ## \e[B\x1b[7D  #### \e[B\x1b[7D",  // Q
    " ####  \e[B\x1b[7D #   # \e[B\x1b[7D ####  \e[B\x1b[7D #  #  \e[B\x1b[7D #   # \e[B\x1b[7D",  // R
    "  #### \e[B\x1b[7D #     \e[B\x1b[7D  ###  \e[B\x1b[7D     # \e[B\x1b[7D ####  \e[B\x1b[7D",  // S
    " ##### \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D",  // T
    " #   # \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D  ###  \e[B\x1b[7D",  // U
    " #   # \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D  # #  \e[B\x1b[7D   #   \e[B\x1b[7D",  // V
    " #   # \e[B\x1b[7D #   # \e[B\x1b[7D # # # \e[B\x1b[7D ## ## \e[B\x1b[7D #   # \e[B\x1b[7D",  // W
    " #   # \e[B\x1b[7D  # #  \e[B\x1b[7D   #   \e[B\x1b[7D  # #  \e[B\x1b[7D #   # \e[B\x1b[7D",  // X
    " #   # \e[B\x1b[7D  # #  \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D",  // Y
    " ##### \e[B\x1b[7D    #  \e[B\x1b[7D   #   \e[B\x1b[7D  #    \e[B\x1b[7D ##### \e[B\x1b[7D",  // Z
    // Números 0-9 (índices 26-35)
    "  ###  \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D #   # \e[B\x1b[7D  ###  \e[B\x1b[7D",  // 0
    "   #   \e[B\x1b[7D  ##   \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D  ###  \e[B\x1b[7D",  // 1
    "  ###  \e[B\x1b[7D     # \e[B\x1b[7D  ###  \e[B\x1b[7D #     \e[B\x1b[7D ##### \e[B\x1b[7D",  // 2
    "  ###  \e[B\x1b[7D     # \e[B\x1b[7D  ###  \e[B\x1b[7D     # \e[B\x1b[7D  ###  \e[B\x1b[7D",  // 3
    " #   # \e[B\x1b[7D #   # \e[B\x1b[7D ##### \e[B\x1b[7D     # \e[B\x1b[7D     # \e[B\x1b[7D",  // 4
    " ##### \e[B\x1b[7D #     \e[B\x1b[7D  ###  \e[B\x1b[7D     # \e[B\x1b[7D  ###  \e[B\x1b[7D",  // 5
    "  ###  \e[B\x1b[7D #     \e[B\x1b[7D ##### \e[B\x1b[7D #   # \e[B\x1b[7D  ###  \e[B\x1b[7D",  // 6
    " ##### \e[B\x1b[7D     # \e[B\x1b[7D    #  \e[B\x1b[7D   #   \e[B\x1b[7D  #    \e[B\x1b[7D",  // 7
    "  ###  \e[B\x1b[7D #   # \e[B\x1b[7D  ###  \e[B\x1b[7D #   # \e[B\x1b[7D  ###  \e[B\x1b[7D",  // 8
    "  ###  \e[B\x1b[7D #   # \e[B\x1b[7D  #### \e[B\x1b[7D     # \e[B\x1b[7D  ###  \e[B\x1b[7D",  // 9
    // Caracteres especiales (índices 36-40)
    "       \e[B\x1b[7D       \e[B\x1b[7D       \e[B\x1b[7D       \e[B\x1b[7D       \e[B\x1b[7D",  // Espacio
    "   #   \e[B\x1b[7D   #   \e[B\x1b[7D   #   \e[B\x1b[7D       \e[B\x1b[7D   #   \e[B\x1b[7D",  // !
    "       \e[B\x1b[7D       \e[B\x1b[7D       \e[B\x1b[7D       \e[B\x1b[7D   #   \e[B\x1b[7D",  // .
    "       \e[B\x1b[7D   #   \e[B\x1b[7D ##### \e[B\x1b[7D   #   \e[B\x1b[7D       \e[B\x1b[7D",  // +
    "       \e[B\x1b[7D       \e[B\x1b[7D ##### \e[B\x1b[7D       \e[B\x1b[7D       \e[B\x1b[7D",  // -
};
