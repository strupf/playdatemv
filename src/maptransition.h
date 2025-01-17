// =============================================================================
// Copyright 2024, Lukas Wolski (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef _MAPTRANSITION_H
#define _MAPTRANSITION_H

#include "gamedef.h"

typedef struct maptransition_s {
    v2_i32 hero_feet;
    v2_i16 hero_v_q8;
    u32    map_hash;
    u8     fade_phase;
    u8     type;
    u8     dir;
    u8     fade_tick;
    i32    teleport_ID;
} maptransition_s;

void   maptransition_teleport(g_s *g, u32 map_hash, v2_i32 hero_feet);
bool32 maptransition_try_hero_slide(g_s *g);
void   maptransition_update(g_s *g);
void   maptransition_draw(g_s *g, v2_i32 cam);

#endif