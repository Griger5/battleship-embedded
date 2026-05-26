#include "battleship.h"
#include <string.h>

static const ShipDef g_ship_defs[NUM_SHIPS] = {
    { "Carrier",    5 },
    { "Battleship", 4 },
    { "Cruiser",    3 },
    { "Submarine",  3 },
    { "Destroyer",  2 }
};

const ShipDef *ship_defs(void) { return g_ship_defs; }

/* ── find which ship occupies (row,col), or -1 ──────────────────── */
static int8_t find_ship_at(const Player *p, uint8_t row, uint8_t col)
{
    for (uint8_t i = 0; i < NUM_SHIPS; i++) {
        const Ship *s = &p->ships[i];
        if (!ship_placed(s)) continue;
        uint8_t sz = g_ship_defs[i].size;
        if (ship_horiz(s)) {
            if (row == s->row && col >= s->col && col < s->col + sz)
                return (int8_t)i;
        } else {
            if (col == s->col && row >= s->row && row < s->row + sz)
                return (int8_t)i;
        }
    }
    return -1;
}

/* ── derive own-board cell for display ───────────────────────────── */
uint8_t own_cell(const Player *p, uint8_t r, uint8_t c)
{
    uint8_t shot = bmp_get(p->attacked, r, c);
    int8_t  sid  = find_ship_at(p, r, c);
    if (sid >= 0)
        return shot ? HIT : SHIP;
    else
        return shot ? MISS : WATER;
}

/* ── init ────────────────────────────────────────────────────────── */
void player_init(Player *p)
{
    memset(p->attacked, 0, sizeof(p->attacked));
    memset(p->track,    0, sizeof(p->track));
    for (uint8_t i = 0; i < NUM_SHIPS; i++) {
        p->ships[i].row   = 0;
        p->ships[i].col   = 0;
        p->ships[i].flags = 0;
    }
    p->ships_placed = 0;
    p->ships_alive  = 0;
}

/* ── placement ───────────────────────────────────────────────────── */
PlaceResult place_ship(Player *p, uint8_t idx,
                       uint8_t row, uint8_t col, uint8_t horizontal)
{
    if (idx >= NUM_SHIPS)       return PLACE_INVALID_INDEX;
    if (p->ships_placed >= NUM_SHIPS) return PLACE_ALL_PLACED;

    uint8_t size = g_ship_defs[idx].size;
    uint8_t dr   = horizontal ? 0 : 1;
    uint8_t dc   = horizontal ? 1 : 0;

    uint8_t end_r = row + dr * (size - 1);
    uint8_t end_c = col + dc * (size - 1);
    if (end_r >= BOARD_SIZE || end_c >= BOARD_SIZE)
        return PLACE_OUT_OF_BOUNDS;

    /* overlap + adjacency — iterate ships instead of reading a board */
    for (uint8_t i = 0; i < size; i++) {
        uint8_t r = row + dr * i;
        uint8_t c = col + dc * i;

        /* overlap: is an existing ship here? */
        if (find_ship_at(p, r, c) >= 0)
            return PLACE_OVERLAP;

        /* adjacency: any existing ship in the 8 neighbours? */
        for (int8_t nr = (int8_t)r - 1; nr <= (int8_t)r + 1; nr++) {
            for (int8_t nc = (int8_t)c - 1; nc <= (int8_t)c + 1; nc++) {
                if (nr < 0 || nr >= BOARD_SIZE ||
                    nc < 0 || nc >= BOARD_SIZE)
                    continue;
                if (nr == (int8_t)r && nc == (int8_t)c)
                    continue;
                if (find_ship_at(p, (uint8_t)nr, (uint8_t)nc) >= 0)
                    return PLACE_ADJACENT;
            }
        }
    }

    /* commit */
    p->ships[idx].row = row;
    p->ships[idx].col = col;
    p->ships[idx].flags = 0;
    ship_set_horiz (&p->ships[idx], horizontal);
    ship_set_placed(&p->ships[idx], 1);
    p->ships_placed++;
    p->ships_alive++;
    return PLACE_OK;
}

void clear_ships(Player *p) { player_init(p); }

uint8_t all_placed(const Player *p)
{
    return p->ships_placed == NUM_SHIPS;
}

/* ── combat ──────────────────────────────────────────────────────── */
ShotResult receive_shot(Player *p, uint8_t row, uint8_t col)
{
    if (row >= BOARD_SIZE || col >= BOARD_SIZE)
        return SHOT_INVALID;

    if (bmp_get(p->attacked, row, col))
        return SHOT_ALREADY;

    bmp_set(p->attacked, row, col);

    int8_t sid = find_ship_at(p, row, col);
    if (sid < 0)
        return SHOT_MISS;

    /* hit */
    uint8_t h = ship_hits(&p->ships[sid]) + 1;
    ship_set_hits(&p->ships[sid], h);

    if (h >= g_ship_defs[sid].size) {
        p->ships_alive--;
        if (p->ships_alive == 0)
            return SHOT_WIN;
        return SHOT_SUNK;
    }
    return SHOT_HIT;
}

void record_shot(Player *p, uint8_t row, uint8_t col, ShotResult result)
{
    if (row >= BOARD_SIZE || col >= BOARD_SIZE)
        return;
    switch (result) {
        case SHOT_MISS:
            board_set(p->track, row, col, MISS);
            break;
        case SHOT_HIT:
        case SHOT_SUNK:
        case SHOT_WIN:
            board_set(p->track, row, col, HIT);
            break;
        default:
            break;
    }
}

uint8_t all_sunk(const Player *p)
{
    return (p->ships_alive == 0) && (p->ships_placed == NUM_SHIPS);
}