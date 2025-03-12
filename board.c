#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

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
    FEAT_MONASTERY,
    FEAT_GARDEN
};

enum alt_feature {
    ALT_GARDEN,
    ALT_FARMHOUSE,
    ALT_COWSHED,
    ALT_TOWER,
    ALT_HIGHWAYMAN,
    ALT_PIGSTY,
    ALT_STABLE
};

struct tile {
    enum edge *sides;
    struct tile **near;
    enum feature feature;
    enum alt_feature alt;
    struct player *player;
    int16_t x;
    int16_t y;
};

struct tile *alloc_tile(enum edge *edges, struct tile **nears, enum feature feature,
            enum alt_feature alt, struct player *player, int16_t x, int16_t y) {
    struct tile *tile;
    tile = malloc(sizeof(struct tile));
    tile->sides = edges;
    tile->near = nears;
    tile->feature = feature;
    tile->alt = alt;
    tile->player = player;
    tile->x = x;
    tile->y = y;
    return tile;
}

void free_tile(struct tile *tile) {
    free(tile->sides);
    free(tile->near);
    free(tile);
}

struct tilestack {
    struct tile **tiles;
    uint16_t count;
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

    ts_push(ts, tile);
}

struct tile *assemble_tile(char *code) {
    uint8_t i;
    enum edge *edges;

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



    return alloc_tile(edges, NULL, NULL, NULL, NULL, 0, 0);
}

struct tilestack *alloc_deck(char *path) {
    struct tilestack *ts;
    ts = malloc(sizeof(struct tilestack));

    return ts;
}

// traversal is going to put 0, 0 at the origin point, and coordinate traversal will be done by linked list 