/* =============================================================================
* Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
* This source code is licensed under the GPLv3 license found in the
* LICENSE file in the root directory of this source tree.
============================================================================= */

#ifndef HERO_H
#define HERO_H

#include "gamedef.h"

enum hero_inp {
        HERO_INP_LEFT  = 0x01,
        HERO_INP_RIGHT = 0x02,
        HERO_INP_UP    = 0x04,
        HERO_INP_DOWN  = 0x08,
        HERO_INP_JUMP  = 0x10,
};

struct hero_s {
        objhandle_s obj;
        i32         jumpticks;
        i32         edgeticks;
        int         inp;  // input mask
        int         inpp; // input mask previous frame
};

obj_s *hero_create(game_s *g, hero_s *h);
void   hero_update(game_s *g, obj_s *o, hero_s *h);

#endif