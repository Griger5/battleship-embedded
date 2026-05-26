#ifndef BATTLESHIP_H
#define BATTLESHIP_H
#include <stdint.h>

#define BOARD_SIZE 12
#define NUM_SHIPS 5
#define TOTAL_CELLS 17 // 5+4+3+3+2
#define BOARD_ROW_BYTES 3 // ceil(BOARD_SIZE/4) for 2-bit boards
#define BITMAP_BYTES 18 // ceil(BOARD_SIZE*BOARD_SIZE / 8)

typedef enum { WATER = 0, SHIP, HIT, MISS }  Cell;

typedef enum {
    SHOT_MISS = 0, SHOT_HIT, SHOT_SUNK,
    SHOT_ALREADY, SHOT_INVALID, SHOT_WIN
} ShotResult;

typedef enum {
    PLACE_OK = 0, PLACE_OUT_OF_BOUNDS, PLACE_OVERLAP,
    PLACE_ADJACENT, PLACE_ALL_PLACED, PLACE_INVALID_INDEX
} PlaceResult;

typedef enum {
    PHASE_SETUP = 0, PHASE_P0_TURN, PHASE_P1_TURN, PHASE_FINISHED
} Phase;

typedef struct { const char *name; uint8_t size; } ShipDef;

typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t flags;
} Ship;

static inline uint8_t ship_horiz(const Ship *s)  { return  s->flags        & 1; }
static inline uint8_t ship_placed(const Ship *s)  { return (s->flags >> 1)  & 1; }
static inline uint8_t ship_hits(const Ship *s)    { return (s->flags >> 2)  & 7; }

static inline void ship_set_horiz(Ship *s, uint8_t v) {
    s->flags = (uint8_t)((s->flags & ~0x01u) | ( v       & 1u));
}
static inline void ship_set_placed(Ship *s, uint8_t v) {
    s->flags = (uint8_t)((s->flags & ~0x02u) | ((v & 1u) << 1));
}
static inline void ship_set_hits(Ship *s, uint8_t v) {
    s->flags = (uint8_t)((s->flags & ~0x1Cu) | ((v & 7u) << 2));
}

typedef struct {
    uint8_t  attacked[BITMAP_BYTES];
    uint8_t  track[BOARD_SIZE][BOARD_ROW_BYTES];
    Ship     ships[NUM_SHIPS];
    uint8_t  ships_placed;
    uint8_t  ships_alive;
} Player;

static inline uint8_t bmp_get(const uint8_t bmp[], uint8_t r, uint8_t c)
{
    uint16_t idx = (uint16_t)r * BOARD_SIZE + c;
    return (bmp[idx >> 3] >> (idx & 7)) & 1;
}
static inline void bmp_set(uint8_t bmp[], uint8_t r, uint8_t c)
{
    uint16_t idx = (uint16_t)r * BOARD_SIZE + c;
    bmp[idx >> 3] |= (uint8_t)(1u << (idx & 7));
}

static inline uint8_t board_get(const uint8_t board[][BOARD_ROW_BYTES],
                                uint8_t r, uint8_t c)
{
    uint8_t bi = c >> 2;
    uint8_t bo = (c & 3) << 1;
    return (board[r][bi] >> bo) & 0x03;
}
static inline void board_set(uint8_t board[][BOARD_ROW_BYTES],
                              uint8_t r, uint8_t c, uint8_t val)
{
    uint8_t bi = c >> 2;
    uint8_t bo = (c & 3) << 1;
    board[r][bi] = (uint8_t)(
        (board[r][bi] & ~(0x03u << bo)) | ((val & 0x03u) << bo));
}

const ShipDef *ship_defs(void);

void        player_init  (Player *p);
PlaceResult place_ship   (Player *p, uint8_t ship_index, uint8_t row, uint8_t col, uint8_t horizontal);
void        clear_ships  (Player *p);
uint8_t     all_placed   (const Player *p);
ShotResult  receive_shot (Player *p, uint8_t row, uint8_t col);
void        record_shot  (Player *p, uint8_t row, uint8_t col, ShotResult result);
uint8_t     all_sunk     (const Player *p);

uint8_t     own_cell     (const Player *p, uint8_t r, uint8_t c);

#endif /* BATTLESHIP_H */