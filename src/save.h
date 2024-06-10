// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef SAVE_H
#define SAVE_H

#include "gamedef.h"

#define INVENTORY_NUM_ITEMS 256
#define NUM_MAP_PINS        64
#define NUM_SAVEIDS         256

typedef struct {
    u16 ID;
    u16 n;
} inventory_item_s;

typedef struct {
    u32              game_version;
    i16              health;
    i16              flytime;
    flags32          upgrades;
    char             name[32];
    i16              coins;
    i16              n_items;
    i16              n_saveIDs;
    i16              n_map_pins;
    inventory_item_s items[INVENTORY_NUM_ITEMS];
    u32              saveIDs[NUM_SAVEIDS]; // unlocked things or events already happened
    char             hero_mapfile[64];
    v2_i32           hero_pos;
    map_pin_s        map_pins[NUM_MAP_PINS];
    u8               visited_tiles[NUM_WORLD_ROOMS][NUM_WORLD_ROOM_TILES >> 3]; // bitset
} save_s;

bool32 hero_has_upgrade(game_s *g, i32 ID);
void   hero_add_upgrade(game_s *g, i32 ID);
void   hero_rem_upgrade(game_s *g, i32 ID);
void   hero_set_name(game_s *g, const char *name);
char  *hero_get_name(game_s *g);
void   hero_inv_add(game_s *g, i32 ID, i32 n);
void   hero_inv_rem(game_s *g, i32 ID, i32 n);
i32    hero_inv_count_of(game_s *g, i32 ID);
void   hero_coins_change(game_s *g, i32 n);
i32    hero_coins(game_s *g);
bool32 hero_visited_tile(game_s *g, map_worldroom_s *room, i32 x, i32 y);
void   hero_set_visited_tile(game_s *g, map_worldroom_s *room, i32 x, i32 y);
void   saveID_put(game_s *g, u32 ID);
bool32 saveID_has(game_s *g, u32 ID);

#endif