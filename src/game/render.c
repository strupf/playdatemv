/* =============================================================================
* Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
* This source code is licensed under the GPLv3 license found in the
* LICENSE file in the root directory of this source tree.
============================================================================= */

#include "render.h"
#include "game.h"
#include "os/os.h"

void draw_tiles(game_s *g, i32 x1, i32 y1, i32 x2, i32 y2)
{
        for (int y = y1; y <= y2; y++) {
                for (int x = x1; x <= x2; x++) {
                }
        }
}

void render_draw(game_s *g)
{
        gfx_rec_fill((rec_i32){50, 70, 30, 40}, 1);
        fnt_s font = fnt_get(FNTID_DEFAULT);
        gfx_text_ascii(&font, "Hello World", 10, 10);

        i32 px1 = g->cam.r.x;
        i32 py1 = g->cam.r.y;
        i32 px2 = g->cam.r.x + g->cam.r.w;
        i32 py2 = g->cam.r.y + g->cam.r.h;

        i32 tx1 = MAX(px1, 0) >> 4;
        i32 ty1 = MAX(py1, 0) >> 4;
        i32 tx2 = MIN(px2, g->pixel_x) >> 4;
        i32 ty2 = MIN(py2, g->pixel_y) >> 4;

        draw_tiles(g, tx1, ty1, tx2, ty2);
}