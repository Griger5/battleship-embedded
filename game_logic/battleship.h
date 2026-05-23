#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include <stdint.h>

#define BOARD_SIZE 12
#define NUM_SHIPS 5
#define TOTAL_CELLS 17
#define BOARD_ROW_BYTES 3

typedef enum {
    WATER = 0,
    SHIP,
    HIT,
    MISS
} Cell;

typedef enum {
    SHOT_MISS = 0,
    SHOT_HIT,
    SHOT_SUNK,
    SHOT_ALREADY,
    SHOT_INVALID,
    SHOT_WIN
} ShotResult;

typedef enum {
    PLACE_OK = 0,
    PLACE_OUT_OF_BOUNDS,
    PLACE_OVERLAP,
    PLACE_ADJACENT,
    PLACE_ALL_PLACED,
    PLACE_INVALID_INDEX
} PlaceResult;

typedef enum {
    PHASE_SETUP = 0,
    PHASE_P0_TURN,
    PHASE_P1_TURN,
    PHASE_FINISHED
} Phase;

typedef struct {
    const char *name;
    uint8_t size;
} ShipDef;

typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t size;
    uint8_t horizontal;
    uint8_t hits;
    uint8_t placed;
} Ship;

typedef struct {
    uint8_t own  [BOARD_SIZE][BOARD_ROW_BYTES];
    uint8_t track[BOARD_SIZE][BOARD_ROW_BYTES];
    Ship ships[NUM_SHIPS];
    uint8_t ships_placed;
    uint8_t ships_alive;
} Player;

static inline uint8_t board_get(const uint8_t board[][BOARD_ROW_BYTES], uint8_t r, uint8_t c) {
    uint8_t byte_idx = c >> 2;
    uint8_t bit_off  = (c & 0x03) << 1;
    return (board[r][byte_idx] >> bit_off) & 0x03;
}

static inline void board_set(uint8_t board[][BOARD_ROW_BYTES], uint8_t r, uint8_t c, uint8_t val) {
    uint8_t byte_idx = c >> 2;
    uint8_t bit_off  = (c & 0x03) << 1;
    board[r][byte_idx] = (uint8_t)((board[r][byte_idx] & ~(0x03 << bit_off)) | ((val & 0x03) << bit_off));
}

const ShipDef *ship_defs(void);

void player_init  (Player *p);

PlaceResult place_ship(Player *p, uint8_t ship_index, uint8_t row, uint8_t col, uint8_t horizontal);

void clear_ships(Player *p);
uint8_t all_placed(const Player *p);

ShotResult receive_shot(Player *p, uint8_t row, uint8_t col);
void record_shot(Player *p, uint8_t row, uint8_t col, ShotResult result);

uint8_t all_sunk(const Player *p);

#endif /* BATTLESHIP_H */