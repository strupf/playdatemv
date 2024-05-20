// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#include "game.h"

typedef struct {
    i32 n_frames;
    u16 t;
    u16 t_og;
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} spritedecal_s;

void spritedecal_on_update(game_s *g, obj_s *o)
{
    spritedecal_s *sd = (spritedecal_s *)o->mem;
    sd->t++;
    if (sd->t_og <= sd->t) {
        obj_delete(g, o);
    }
}

void spritedecal_on_animate(game_s *g, obj_s *o)
{
    spritedecal_s   *sd     = (spritedecal_s *)o->mem;
    sprite_simple_s *spr    = &o->sprites[0];
    obj_s           *parent = obj_from_obj_handle(o->obj_handles[0]);
    if (parent) {
        o->pos = parent->pos;
    }
    spr->trec.r.x = sd->x + sd->w * ((sd->t * sd->n_frames) / sd->t_og);
}

obj_s *spritedecal_create(game_s *g, i32 render_priority, obj_s *oparent, v2_i32 pos,
                          i32 texID, rec_i32 srcr, i32 ticks, i32 n_frames, int flip)
{
    obj_s *o             = obj_create(g);
    o->ID                = OBJ_ID_SPRITEDECAL;
    o->flags             = OBJ_FLAG_SPRITE;
    o->n_sprites         = 1;
    o->render_priority   = render_priority;
    o->on_animate        = spritedecal_on_animate;
    o->on_update         = spritedecal_on_update;
    sprite_simple_s *spr = &o->sprites[0];
    spr->flip            = flip;
    if (oparent) {
        o->obj_handles[0] = obj_handle_from_obj(oparent);
        spr->offs         = pos;
    } else {
        o->pos = pos;
    }

    spritedecal_s *sd = (spritedecal_s *)o->mem;
    sd->n_frames      = n_frames;
    sd->x             = srcr.x;
    sd->y             = srcr.y;
    sd->w             = srcr.w;
    sd->h             = srcr.h;
    sd->t_og          = ticks;

    spr->trec = asset_texrec(texID, sd->x, sd->y, sd->w, sd->h);
    return o;
}