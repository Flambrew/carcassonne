#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEFAULT_STACK_ALLOCATION 12
#define TILE_CODE_LEN 6
#define NAME_LEN 12

struct player {
    uint8_t ident;
    char name[NAME_LEN];
};

void set_gameseed(struct player **players, uint8_t count) {
    int i, j, seed;
    for (i = seed = 0; i < count; ++i) {
        for (j = 0; j < NAME_LEN; ++j) {
            seed = (seed + i) * (players[i]->name[j] + j);
        }
    }
    srand(seed);
}

enum edge {
    NULL_EDGE,
    EDGE_GRASS,
    EDGE_ROAD,
    EDGE_CITY,
    EDGE_RIVER
};

enum feature {
    NULL_FEATURE,
    FEAT_VILLAGE,    
    FEAT_CITY,
    FEAT_SHIELD,
    FEAT_MONASTERY
};

enum alt_feature {
    NULL_ALT,
    ALT_GARDEN,
    ALT_FARMHOUSE,
    ALT_TOWER,
    ALT_HIGHWAYMAN,
    ALT_STABLE,
    ALT_SOURCE,
    ALT_LAKE
};

struct tile {
    enum edge *sides;
    enum feature feature;
    enum alt_feature alt;
    struct player *player;
    struct tile **near;
    int16_t x;
    int16_t y;
};

struct tile *alloc_tile(char *code) {
    uint8_t i;
    enum edge *edges;
    enum feature feature;
    enum alt_feature alt;
    struct tile *tile;

    edges = malloc(4 * sizeof(enum edge));
    for (i = 0; i < 4; ++i) {
        if (code[i] == 'g') {
            edges[i] = EDGE_GRASS;
        } else if (code[i] == 'r') {
            edges[i] = EDGE_ROAD;
        } else if (code[i] == 'c') {
            edges[i] = EDGE_CITY;
        } else if (code[i] == 's') {
            edges[i] = EDGE_RIVER;
        }
    }

    if (code[4] == 'x') {
        feature = NULL_FEATURE;
    } else if (code[4] == 'v') {
        feature = FEAT_VILLAGE;
    } else if (code[4] == 'c') {
        feature = FEAT_CITY;
    } else if (code[4] == 's') {
        feature = FEAT_SHIELD;
    } else if (code[4] == 'm') {
        feature = FEAT_MONASTERY;
    }

    if (code[5] == 'x') {
        alt = NULL_ALT;
    } else if (code[5] == 'g') {
        alt = ALT_GARDEN;
    } else if (code[5] == 'f') {
        alt = ALT_FARMHOUSE;
    } else if (code[5] == 't') {
        alt = ALT_TOWER;
    } else if (code[5] == 'h') {
        alt = ALT_HIGHWAYMAN;
    } else if (code[5] == 's') {
        alt = ALT_STABLE;
    }

    tile = malloc(sizeof(struct tile));
    tile->sides = edges;
    tile->feature = feature;
    tile->alt = alt;
    tile->player = NULL;
    tile->near = NULL;
    tile->x = 0;
    tile->y = 0;
    return tile;
}

void free_tile(struct tile *tile) {
    free(tile->sides);
    if (tile->near != NULL) {
        free(tile->near);
    }

    free(tile);
}

struct tilestack {
    struct tile **tiles;
    uint16_t count;
    uint16_t capacity;
};

struct tile *ts_pop(struct tilestack *ts) {
    uint8_t i;
    struct tile *curr = ts->tiles[--ts->count];
    for (i = 0; i < 4; ++i) 
        if (curr->near[i] != NULL) 
            curr->near[i]->near[(i + 2) % 4] = NULL;
    return curr;
}

void ts_push(struct tilestack *ts, struct tile *tile) {
    if (ts->count == ts->capacity) {
        ts->tiles = realloc(ts->tiles, (ts->capacity *= 2) * sizeof(struct tile *));
    }

    ts->tiles[ts->count++] = tile;
}

struct tile *ts_get(struct tilestack *ts, int16_t x, int16_t y) {
    uint16_t i; 
    for (i = 0; i < ts->count; ++i)
        if (ts->tiles[i]->x == x && ts->tiles[i]->y == y)
            return ts->tiles[i];
    return NULL;
}

bool ts_placeat(struct tilestack *ts, struct tile *tile, int16_t x, int16_t y) {
    uint8_t i;
    struct tile *near[4];
    near[0] = ts_get(ts, x, y + 1);
    near[1] = ts_get(ts, x + 1, y);
    near[2] = ts_get(ts, x, y - 1);
    near[3] = ts_get(ts, x - 1, y);

    if (near[0] == NULL && near[1] == NULL && near[2] == NULL && near[3] == NULL) {
        return false;
    }

    for (i = 0; i < 4; ++i) {
        if (near[i] != NULL && tile->sides[i] != near[i]->sides[(i + 2) % 4]) {
            return false;
        }
    }

    for (i = 0; i < 4; ++i) {
        near[i]->near[(i + 2) % 4] = tile;
    }

    tile->x = x;
    tile->y = y;

    ts_push(ts, tile);

    // TODO: propogate completions

    return true;
}

struct tilestack *alloc_deck(char *path) {
    struct tilestack *deck;
    char c, code[6];
    FILE *file;
    uint8_t i;

    deck = malloc(sizeof(struct tilestack));
    deck->count = 0;
    deck->capacity = DEFAULT_STACK_ALLOCATION;
    deck->tiles = malloc(deck->capacity * sizeof(struct tile *));

    file = fopen(path, "r");
    while (true) {
        if ((c = getc(file)) == EOF) {
            break;
        }

        for (i = 0; c != ' '; ++i, c = getc(file)) {
            code[i] = c; 
        }

        ts_push(deck, alloc_tile(code));
    }

    return deck;
}

void run() {
    printf("all kosher\n");
}

int main() {
    struct tilestack *ts = alloc_deck("tilesets/base.carc");
    run();
    printf("teehee\n");
}

// TODO alphabetize things
// TODO print errors