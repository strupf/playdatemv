// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef TRANSITION_H
#define TRANSITION_H

#include "cam.h"
#include "game_def.h"

enum {
        TRANSITION_TICKS       = 15,
        TRANSITION_BLACK_TICKS = 10,
};

enum {
        TRANSITION_NONE,
        TRANSITION_FADE_OUT,
        TRANSITION_FADE_BLACK,
        TRANSITION_FADE_IN,
};

struct transition_s {
        int    phase;
        int    ticks;
        char   map[64]; // next map to load
        v2_i32 teleportto;
        int    dir_slide;
};

void transition_update(game_s *g);
void transition_start(game_s *g, char *filename, v2_i32 location, int dir_slide);

#endif