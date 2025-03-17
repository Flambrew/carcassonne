#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_STACK_ALLOCATION 12
#define TILE_CODE_LEN 6
#define NAME_LEN 12

#define BASE "tilesets/base.carc"
#define RIVER "tilesets/river.carc"

#define ASPECT_Y 3
#define ASPECT_X 7
#define PIXELS_PER_TILE 12
#define DOTS_PER_PIXEL 2

#define TEXT_COLOR_SPECIFIER_LEN 8
#define FOREGROUND_TEXT_COLOR 30
#define BACKGROUND_TEXT_COLOR 40

// -----=====<<<<< PLAYER >>>>>=====----- //

typedef struct player {
    uint8_t ident;
    char name[NAME_LEN];
} Player;

void set_gameseed(Player **players, uint8_t count) {
    int i, j, seed;
    for (i = seed = 0; i < count; ++i) {
        for (j = 0; j < NAME_LEN; ++j) {
            seed = (seed + i) * (players[i]->name[j] + j);
        }
    }

    srand(seed);
}

// -----=====<<<<< TILE >>>>>=====----- //

typedef enum edge {
    NULL_EDGE,
    EDGE_CITY,
    EDGE_GRASS,
    EDGE_RIVER,
    EDGE_ROAD,
} Edge;

typedef enum feature {
    NULL_FEATURE, 
    FEAT_CITY,
    FEAT_MONASTERY,
    FEAT_SHIELD,
    FEAT_VILLAGE
} Feature;

typedef enum alt_feature {
    NULL_ALT,
    ALT_FARMHOUSE,
    ALT_GARDEN,
    ALT_HIGHWAYMAN,
    ALT_LAKE,
    ALT_SOURCE,
    ALT_STABLE,
    ALT_TOWER
} Alt_Feature;

typedef struct tile {
    Edge *edges;
    Feature feature;
    Alt_Feature alt;
    Player *player;
    struct tile **near;
    int16_t y, x;
} Tile;

void rotate_tile(Tile *tile, bool left) {
    Edge temp;
    temp = tile->edges[0];
    if (left) {
        tile->edges[0] = tile->edges[1];
        tile->edges[1] = tile->edges[2];
        tile->edges[2] = tile->edges[3];
        tile->edges[3] = temp;
    } else {
        tile->edges[0] = tile->edges[3];
        tile->edges[3] = tile->edges[2];
        tile->edges[2] = tile->edges[1];
        tile->edges[1] = temp;
    }
}

Tile *alloc_tile(char *code) {
    uint8_t i;
    Edge *edges;
    Feature feature;
    Alt_Feature alt;
    Tile *tile;

    edges = (Edge *) malloc(4 * sizeof(Edge));
    for (i = 0; i < 4; ++i) {
        if (code[i] == 'c') {
            edges[i] = EDGE_CITY;
        } else if (code[i] == 'g') {
            edges[i] = EDGE_GRASS;
        } else if (code[i] == 's') {
            edges[i] = EDGE_RIVER;
        } else if (code[i] == 'r') {
            edges[i] = EDGE_ROAD;
        } else {
            return NULL;
        }
    }

    if (code[4] == 'c') {
        feature = FEAT_CITY;
    } else if (code[4] == 'x') {
        feature = NULL_FEATURE;
    } else if (code[4] == 'm') {
        feature = FEAT_MONASTERY;
    } else if (code[4] == 's') {
        feature = FEAT_SHIELD;
    } else if (code[4] == 'v') {
        feature = FEAT_VILLAGE;
    } else {
        return NULL;
    }

    if (code[5] == 'x') {
        alt = NULL_ALT;
    } else if (code[5] == 'f') {
        alt = ALT_FARMHOUSE;
    } else if (code[5] == 'g') {
        alt = ALT_GARDEN;
    } else if (code[5] == 'h') {
        alt = ALT_HIGHWAYMAN;
    } else if (code[5] == 'l') {
        alt = ALT_LAKE;
    } else if (code[5] == 'r') {
        alt = ALT_SOURCE;
    } else if (code[5] == 's') {
        alt = ALT_STABLE;
    } else if (code[5] == 't') {
        alt = ALT_TOWER;
    } else {
        return NULL;
    }

    tile = (Tile *) malloc(sizeof(Tile));
    tile->edges = edges;
    tile->feature = feature;
    tile->alt = alt;
    tile->player = NULL;
    tile->near = NULL;
    tile->x = 0;
    tile->y = 0;
    return tile;
}

void free_tile(Tile *tile) {
    free(tile->edges);
    if (tile->near != NULL) {
        free(tile->near);
    }

    free(tile);
}

// -----=====<<<<< STACK >>>>>=====----- //

typedef struct tile_stack {
    Tile **tiles;
    uint16_t count, capacity;
    int16_t *bounds;
} Tile_Stack;

Tile *ts_draw(Tile_Stack *deck) {
    uint8_t i;
    Tile *curr = deck->tiles[--deck->count];
    for (i = 0; i < 4; ++i) {
        if (curr->near[i] != NULL) {
            curr->near[i]->near[(i + 2) % 4] = NULL; 
        }
    }
    
    return curr;
}

void ts_push(Tile_Stack *ts, Tile *tile) {
    if (ts->count == ts->capacity) {
        ts->tiles = (Tile **) realloc(ts->tiles, (ts->capacity *= 2) * sizeof(Tile *));
    }

    ts->tiles[ts->count++] = tile;
}

Tile *ts_get(Tile_Stack *ts, int16_t y, int16_t x) {
    uint16_t i; 
    for (i = 0; i < ts->count; ++i) {
        if (ts->tiles[i]->y == y && ts->tiles[i]->x == x) {
            return ts->tiles[i];
        }
    }
    
    return NULL;
}

Tile_Stack *alloc_deck(int count, ...) {
    va_list paths;
    va_start(paths, count);

    Tile_Stack *deck;
    char c, code[6], *path;
    FILE *file;
    uint8_t i, j, k;

    deck = (Tile_Stack *) malloc(sizeof(Tile_Stack));
    deck->count = 0;
    deck->capacity = DEFAULT_STACK_ALLOCATION;
    deck->tiles = (Tile **) malloc(deck->capacity * sizeof(Tile *));
    deck->bounds = (int16_t *) malloc(4 * sizeof(int16_t));
    for (i = 0; i < 4; ++i) {
        deck->bounds = 0;
    }

    for (i = 0; i < count; ++i) {
        path = va_arg(paths, char *);
        file = fopen(path, "r");
        for (j = 0; true; ++j) {
            if ((c = getc(file)) == EOF) {
                break;
            }

            for (k = 0; c != ' '; ++k, c = getc(file)) {
                code[k] = c; 
            }

            Tile *tile = alloc_tile(code);
            if (tile == NULL) {
                printf("Improper tile code \"%.6s\" at tile %d in file: %s.", code, j, path);
            }
            ts_push(deck, tile);
        }

        fclose(file);
    }

    va_end(paths);
    return deck;
}

// -----=====<<<<< RADIAL >>>>>=====----- //

typedef struct coordinate {
    int16_t y, x;
    struct coordinate *prev, *next;
} Coordinate;

typedef struct radial_coordinate_list {
    Coordinate *head;
    uint16_t length, selected;
} Radial_Coordinate_List;

Radial_Coordinate_List *alloc_rcl() {
    Radial_Coordinate_List *list = malloc(sizeof(Radial_Coordinate_List));
    list->head = NULL;
    list->length = list->selected = 0;
    return list;
}

bool free_rcl(Radial_Coordinate_List *list) {
    int i;
    Coordinate *curr, *next;
    if (list != NULL) {
        curr = list->head;
        for (i = 0; i < list->length; ++i) {
            next = curr->next;
            free(curr);
            curr = next;
        }

        free(list);
        return true;
    }

    return false;
}

void rcl_add_alloc(Radial_Coordinate_List *list, int16_t y, int16_t x) {
    uint16_t i, atan;
    Coordinate *curr, *new;
    new = malloc(sizeof(Coordinate));
    new->y = y;
    new->x = x;

    if (list->head == NULL) {
        new->next = new->prev = new;
        list->head = new;
        list->length = 1;
    }

    atan = atan2(y, x);
    list->length += 1;
    for (i = 0; i < list->length; ++i, curr = curr->next) {
        if (atan < atan2(curr->y, curr->x)) {
            curr->prev->next = new;
            new->prev = curr->prev;
            curr->prev = new;
            new->next = curr; 
            break;
        }
    }

    if (new->next == NULL) {
        curr->prev->next = new;
        new->prev = curr->prev;
        curr->prev = new;
        new->next = curr;  
    }

    if (atan < atan2(list->head->y, list->head->x)) {
        list->head = new;
    }
}

bool rcl_remove_free(Radial_Coordinate_List *list, int16_t y, int16_t x) {
    uint16_t i;
    Coordinate *curr;
    curr = list->head;
    for (i = 0; i < list->length; ++i, curr = curr->next) {
        if (curr->y == y && curr->x == x) {
            if (curr == list->head) {
                list->head = curr->next;
            }

            curr->prev->next = curr->next;
            curr->next->prev = curr->prev;
            free(curr);
            return true;
        }
    }
    return false;
}

// -----=====<<<<< TABLE >>>>>=====----- //

typedef struct table {
    Player *players;
    Tile_Stack *deck, *board;
    Radial_Coordinate_List *available;
} Table;

bool place_tile(Table *table, Tile *tile, Player *player, int16_t y, int16_t x) {
    uint8_t i;
    Tile *near[4];
    Tile_Stack *board;
    board = table->board;

    near[0] = ts_get(board, y + 1, x);
    near[1] = ts_get(board, y, x + 1);
    near[2] = ts_get(board, y - 1, x);
    near[3] = ts_get(board, y, x - 1);

    for (i = 0; i < 4; ++i) {
        if (near[i] != NULL && tile->edges[i] != near[i]->edges[(i + 2) % 4]) {
            return false;
        }
    }

    for (i = 0; i < 4; ++i) {
        near[i]->near[(i + 2) % 4] = tile;
    }

    if (board->bounds[0] < y) {
        board->bounds[0] = y;
    } else if (board->bounds[2] > y) {
        board->bounds[2] = y;
    }

    if (board->bounds[1] < x) {
        board->bounds[1] = x;
    } else if (board->bounds[3] > x) {
        board->bounds[3] = x;
    }

    tile->y = y;
    tile->x = x;
    tile->player = player;

    ts_push(board, tile);
    rcl_remove_free(table->available, y, x);

    // TODO: propogate completions

    return true;
}