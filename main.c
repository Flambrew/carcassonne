#include <stdbool.h>

#include "board.h"
#include "defs.h"

int main() {
    // set_locale();
    set_gameseed(NULL, 0);

    Tile_Stack *deck = alloc_deck(2, BASE, RIVER);
    // Viewport *display = alloc_viewport(3);
    // Wchar_Buffer *txtbuf = alloc_wchar_buffer();
}