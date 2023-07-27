/* =============================================================================
* Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
* This source code is licensed under the GPLv3 license found in the
* LICENSE file in the root directory of this source tree.
============================================================================= */

#include "game.h"

void game_load_map(game_s *g, const char *filename)
{
        NOT_IMPLEMENTED

        int w = 1;
        int h = 1;

        g->tiles_x = w;
        g->tiles_y = h;
        g->pixel_x = w << 4;
        g->pixel_y = h << 4;
}