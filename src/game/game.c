/* =============================================================================
* Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
* This source code is licensed under the GPLv3 license found in the
* LICENSE file in the root directory of this source tree.
============================================================================= */

#include "game.h"

void game_init(game_s *g)
{
        tex_put(TEXID_FONT_DEFAULT, tex_load("assets/font_mono_8.json"));
        tex_put(TEXID_TILESET, tex_load("assets/testtiles.json"));
        fnt_s font1      = {0};
        font1.gridw      = 16;
        font1.gridh      = 32;
        font1.lineheight = 28;
        font1.tex        = tex_get(TEXID_FONT_DEFAULT);
        for (int n = 0; n < 256; n++) {
                font1.glyph_widths[n] = 14;
        }
        fnt_put(FNTID_DEFAULT, font1);

        for (int n = 1; n < NUM_OBJS; n++) { // obj at index 0 is "dead"
                obj_s *o               = &g->objs[n];
                o->index               = n;
                o->gen                 = 1;
                g->objfreestack[n - 1] = o;
        }
        g->n_objfree = NUM_OBJS - 1;

        g->cam.r.w = 400;
        g->cam.r.h = 240;

        obj_s *player = obj_create(g);
        player->pos.x = 10;
        player->pos.y = 20;
        player->w     = 16;
        player->h     = 32;
        g->player     = objhandle_from_obj(player);

        game_load_map(g, "assets/samplemap.tmj");
}

void game_update(game_s *g)
{
        obj_s *o;
        if (objhandle_try_dereference(g->player, &o)) {
                if (debug_inp_up()) {
                        o->pos.y--;
                }
                if (debug_inp_down()) {
                        o->pos.y++;
                }
                if (debug_inp_left()) {
                        o->pos.x--;
                }
                if (debug_inp_right()) {
                        o->pos.x++;
                }
        }
}

void game_draw(game_s *g)
{
        render_draw(g);
}

void game_close(game_s *g)
{
}

tilegrid_s game_tilegrid(game_s *g)
{
        tilegrid_s tg = {g->tiles,
                         g->tiles_x,
                         g->tiles_y,
                         g->pixel_x,
                         g->pixel_y};
        return tg;
}