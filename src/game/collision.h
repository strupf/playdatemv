// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef COLLISION_H
#define COLLISION_H

#include "gamedef.h"

// we operate on 16x16 tiles
struct tilegrid_s {
        u8 *tiles; // tileIDs
        int tiles_x;
        int tiles_y;
        int pixel_x;
        int pixel_y;
};

extern const tri_i32 tilecolliders[];

bool32 tiles_area(tilegrid_s tg, rec_i32 r);
bool32 tiles_at(tilegrid_s tg, i32 x, i32 y);

#endif