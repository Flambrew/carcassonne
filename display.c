#include <locale.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "defs.h"
#include "board.h"

void set_locale() {
    setlocale(LC_CTYPE, "");
}

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

Wchar_Buffer *alloc_wchar_buffer() {
    Wchar_Buffer *buf;
    buf = (Wchar_Buffer *) malloc(sizeof(Wchar_Buffer));
    buf->capacity = DEFAULT_STACK_ALLOCATION;
    buf->buffer = (wchar_t *) malloc(buf->capacity * sizeof(wchar_t));
    buf->length = 0;
    return buf;
}

void append_text_color_change(Wchar_Buffer *buffer, Terminal_Color text_color, Terminal_Color bg_color) {
    uint8_t i;
    wchar_t specifier[TEXT_COLOR_SPECIFIER_LEN + 1];
    swprintf(specifier, TEXT_COLOR_SPECIFIER_LEN + 1, L"\033[%d;%dm", text_color + FOREGROUND_TEXT_COLOR, bg_color + BACKGROUND_TEXT_COLOR);
    for (i = 0; i < TEXT_COLOR_SPECIFIER_LEN; ++i) {
        buffer->buffer[buffer->length++] = specifier[i];
    }
}

typedef struct viewport {
    uint8_t *pixels;
    int16_t height, width;
} Viewport;

Viewport *alloc_viewport(uint8_t size) {
    Viewport *vp;
    vp = (Viewport *) malloc(sizeof(Viewport));
    vp->height = PIXELS_PER_TILE * ASPECT_Y * DOTS_PER_PIXEL * (2 * size - 1);
    vp->width = PIXELS_PER_TILE * ASPECT_X * DOTS_PER_PIXEL * (2 * size - 1);
    vp->pixels = (uint8_t *) malloc(vp->height * vp->width * sizeof(uint8_t));
    return vp;
}

void cam_pos(Viewport *vp, Tile_Stack *ts, Tile *tile, int16_t *y, int16_t *x) {
    int16_t y_center, y_quart_diff, x_center, x_quart_diff;
    
    y_center = (ts->bounds[1] + ts->bounds[3]) / 2;
    x_center = (ts->bounds[0] + ts->bounds[2]) / 2;
    y_quart_diff = abs(tile->y - y_center) - vp->height / PIXELS_PER_TILE / 4;
    x_quart_diff = abs(tile->x - x_center) - vp->width / PIXELS_PER_TILE / 4;

    if (y_quart_diff <= 0) {
        *y = y_center;
    } else if (tile->y > y_center) {
        *y = tile->y - y_quart_diff;
    } else {
        *y = tile->y + y_quart_diff;
    }

    if (x_quart_diff <= 0) {
        *x = x_center;
    } else if (tile->x > x_center) {
        *x = tile->x - x_quart_diff;
    } else {
        *x = tile->x + x_quart_diff;
    }
}

wchar_t *viewport_charbuf(Viewport *vp) {
    /*wchar_t *out;
    uint16_t i, j;
    out = (wchar_t *) malloc(vp->height / 2 * vp->width * sizeof(wchar_t));
    for (i = 0; i < vp->height / 2; ++i) {
        for (j = 0; j < vp->width; ++j) {
            out[i * vp->width + j] = 0x2800 +
                    (vp->pixels[(i * 2)     * vp->width + j] ? 1 : 0) * 23 +
                    (vp->pixels[(i * 2 + 1) * vp->width + j] ? 1 : 0) * 232;
        }
    }*/
    return NULL;
}

// wprintf(L"%lc", 0);

// TODO figure out graphics
