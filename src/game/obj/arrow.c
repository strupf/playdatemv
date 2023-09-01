// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#include "game/game.h"

void arrow_think(game_s *g, obj_s *o, void *arg)
{
        rec_i32 aabb = obj_aabb(o);
        aabb.x--;
        aabb.y--;
        aabb.w += 2;
        aabb.h = 2;
        if (game_area_blocked(g, aabb)) {
                obj_delete(g, o);
                return;
        }

        if (o->vel_q8.x != 0) {
                o->facing = SGN(o->vel_q8.x);
        }
}

obj_s *arrow_create(game_s *g, v2_i32 p, v2_i32 v_q8)
{
        obj_s     *arrow = obj_create(g);
        objflags_s flags = objflags_create(
            OBJ_FLAG_ACTOR,
            OBJ_FLAG_MOVABLE,
            OBJ_FLAG_THINK_1,
            OBJ_FLAG_KILL_OFFSCREEN);
        obj_set_flags(g, arrow, flags);
        arrow->think_1      = arrow_think;
        arrow->ID           = 2;
        arrow->w            = 8;
        arrow->h            = 8;
        arrow->pos.x        = p.x - arrow->w / 2;
        arrow->pos.y        = p.y - arrow->h / 2;
        arrow->facing       = SGN(v_q8.x);
        arrow->vel_q8       = v_q8;
        arrow->gravity_q8.y = 12;
        arrow->drag_q8.x    = 256;
        arrow->drag_q8.y    = 256;
        arrow->onsqueeze    = obj_squeeze_delete;
        return arrow;
}