/* =============================================================================
* Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
* This source code is licensed under the GPLv3 license found in the
* LICENSE file in the root directory of this source tree.
============================================================================= */

#include "render.h"
#include "game.h"
#include "os/os.h"

void draw_textbox(game_s *g)
{
}

void draw_tiles(game_s *g, i32 x1, i32 y1, i32 x2, i32 y2)
{
        ASSERT(0 <= x1 && 0 <= y1 && x2 < g->tiles_x && y2 < g->tiles_y);

        tex_s tileset = tex_get(TEXID_TILESET);
        for (int y = y1; y <= y2; y++) {
                v2_i32 pos;
                pos.y = (y << 4) - g->cam.r.y;
                for (int x = x1; x <= x2; x++) {
                        pos.x           = (x << 4) - g->cam.r.x;
                        int      tID    = x + y * g->tiles_x;
                        rtile_s *rtiles = g->rtiles[tID];

                        for (int n = 0; n < 1; n++) {   // actually for both layers
                                rtile_s rt = rtiles[n]; // todo
                                if (rt.flags == 0xFF) continue;
                                int     id      = rt.ID;
                                rec_i32 tilerec = {(id & 15) << 4,
                                                   (id >> 4) << 4,
                                                   16, 16};
                                gfx_sprite(tileset, pos, tilerec, rt.flags);
                        }
                }
        }
}

void render_draw(game_s *g)
{
        obj_s *player;
        if (try_obj_from_handle(g->hero.obj, &player)) {
                g->cam.r.x = player->pos.x - g->cam.r.w / 2;
                g->cam.r.y = player->pos.y - g->cam.r.h / 2;
        }

        i32 px1 = g->cam.r.x;
        i32 py1 = g->cam.r.y;
        i32 px2 = g->cam.r.x + g->cam.r.w;
        i32 py2 = g->cam.r.y + g->cam.r.h;
        i32 tx1 = MAX(px1, 0) >> 4;
        i32 ty1 = MAX(py1, 0) >> 4;
        i32 tx2 = MIN(px2, g->pixel_x - 1) >> 4;
        i32 ty2 = MIN(py2, g->pixel_y - 1) >> 4;

        draw_tiles(g, tx1, ty1, tx2, ty2);

        for (int n = 0; n < objset_len(&g->obj_active); n++) {
                obj_s  *o = objset_at(&g->obj_active, n);
                rec_i32 r = translate_rec_xy(obj_aabb(o),
                                             -g->cam.r.x,
                                             -g->cam.r.y);
                gfx_rec_fill(r, 1);
        }

        if (g->textbox.active) {
                draw_textbox(g);
        }

        for (ropenode_s *r1 = g->rope.head; r1 && r1->next; r1 = r1->next) {
                ropenode_s *r2 = r1->next;
                gfx_line(r1->p.x - g->cam.r.x, r1->p.y - g->cam.r.y,
                         r2->p.x - g->cam.r.x, r2->p.y - g->cam.r.y, 1);
        }

        // simple transition animation
        if (g->transitionphase) {
                switch (g->transitionphase) {
                case TRANSITION_FADE_IN: {
                        int x = lerp_i32(0, 410,
                                         g->transitionticks,
                                         TRANSITION_TICKS);
                        gfx_rec_fill((rec_i32){0, 0, x, 240}, 1);
                } break;
                case TRANSITION_FADE_OUT: {
                        int x = lerp_i32(410, 0,
                                         g->transitionticks,
                                         TRANSITION_TICKS);
                        gfx_rec_fill((rec_i32){0, 0, x, 240}, 1);
                } break;
                }
        }
}