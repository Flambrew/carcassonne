#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "board.h"

void set_locale();

typedef enum terminal_color {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE
} Terminal_Color;

typedef struct wchar_buffer {
    wchar_t *buffer;
    uint32_t length, capacity;
} Wchar_Buffer;

Wchar_Buffer *alloc_wchar_buffer();
void append_text_color_change(Wchar_Buffer *buffer, Terminal_Color text_color, Terminal_Color bg_color);

typedef struct viewport {
    uint8_t *pixels;
    int16_t height, width;
} Viewport;

Viewport *alloc_viewport(uint8_t size);
void cam_pos(Viewport *vp, Tile_Stack *ts, Tile *tile, int16_t *y, int16_t *x);
wchar_t *viewport_charbuf(Viewport *vp);