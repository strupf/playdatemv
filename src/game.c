// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#include "game.h"
#include "render.h"

void game_tick_gameplay(game_s *g);

void game_init(game_s *g)
{
    mus_set_trg_vol(1.f);
    g->cam.mode = CAM_MODE_FOLLOW_HERO;

    map_world_load(&g->map_world, "world.world");
#if LIGHTING_ENABLED
    lighting_init(&g->lighting);
    g->lighting.n_lights    = 1;
    g->lighting.lights[0].r = 100;
#endif
    pltf_log("GAME VERSION %u\n", GAME_VERSION);

    g->n_deco_verlet = 1;
    for (u32 k = 0; k < g->n_deco_verlet; k++) {
        g->deco_verlet[k].n_pt     = 10;
        g->deco_verlet[k].dist     = 128 << 2;
        g->deco_verlet[k].n_it     = 3;
        g->deco_verlet[k].grav.y   = 8 << 2;
        g->deco_verlet[k].haspos_2 = 1;
        g->deco_verlet[k].pos_2    = (v2_i16){64 << 6, 32};
        g->deco_verlet[k].pos      = (v2_i32){64 + k * 16, 32};
        for (u32 n = 0; n < g->deco_verlet[k].n_pt; n++) {
            g->deco_verlet[k].pt[n].p  = (v2_i16){0};
            g->deco_verlet[k].pt[n].pp = (v2_i16){0};
        }
    }
}

void game_tick(game_s *g)
{
    bool32 update_gameplay = 0;
    if (g->aud_lowpass) {
        g->aud_lowpass--;
        aud_set_lowpass(((g->aud_lowpass * 12) / 30));
    }

    if (g->substate) {
        g->freeze_tick        = 0;
        g->item_select.docked = 0;
    } else {
        if (0 < g->freeze_tick) {
            g->freeze_tick--;
        } else {
            game_tick_gameplay(g);
        }
    }

    switch (g->substate) {
    case SUBSTATE_TEXTBOX:
        textbox_update(g);
        break;
    case SUBSTATE_MAPTRANSITION:
        maptransition_update(g);
        break;
    case SUBSTATE_GAMEOVER:
        gameover_update(g);
        break;
    }

    cam_update(g, &g->cam);
    if (g->areaname.fadeticks) {
        g->areaname.fadeticks++;
        if (FADETICKS_AREALABEL <= g->areaname.fadeticks) {
            g->areaname.fadeticks = 0;
        }
    }

    // save animation
    if (g->save_ticks) {
        g->save_ticks++;
        if (SAVE_TICKS <= g->save_ticks) {
            g->save_ticks = 0;
        }
    }

    // every other tick to save some CPU cycles;
    // split between even and uneven frames
    if (g->gameplay_tick & 1) {
        grass_animate(g);
    } else {
    }

    deco_verlet_animate(g);
    area_update(g, &g->area);
}

void game_tick_gameplay(game_s *g)
{
    g->gameplay_tick++;
    item_select_update(&g->item_select);
    g->events_frame          = 0;
    g->hero_mem.interactable = obj_handle_from_obj(NULL);

    for (obj_each(g, o)) {
        v2_i32 posprev = o->pos;

        if (o->on_update) {
            o->on_update(g, o);
        }

        o->posprev = posprev;

        if (0 < o->invincible_tick) {
            o->invincible_tick--;
        }

        if (o->enemy.hurt_tick) {
            o->enemy.hurt_tick--;
        }
    }

    for (obj_each(g, o)) { // integrate acc, vel and drag: adds tomove accumulator
        if (o->flags & OBJ_FLAG_MOVER) {
            obj_apply_movement(o);
        }
    }

    for (obj_each(g, o)) {
        o->moverflags |= OBJ_MOVER_MAP;
        if (!obj_try_wiggle(g, o)) continue;
        obj_move(g, o, o->tomove);
        o->tomove.x = 0, o->tomove.y = 0;
    }

    // out of bounds deletion
    rec_i32 roombounds = {0, 0, g->pixel_x, g->pixel_y};
    for (obj_each(g, o)) {
        if ((o->flags & OBJ_FLAG_KILL_OFFSCREEN) &&
            !overlap_rec(obj_aabb(o), roombounds)) {
            obj_delete(g, o);
        }
    }

    objs_cull_to_delete(g);
    coinparticle_update(g);

    if (g->rope.active) {
        rope_update(g, &g->rope);
        rope_verletsim(g, &g->rope);
    }

    obj_s *ohero = obj_get_tagged(g, OBJ_TAG_HERO);
    if (ohero) {
        if (ohero->health) {
            hero_post_update(g, ohero);
        } else {
        }
    }

#ifdef PLTF_DEBUG
    for (obj_each(g, o)) {
        assert(o->magic == OBJ_MAGIC);
    }
#endif

    if (g->events_frame & EVENT_HERO_DAMAGE) {
        g->freeze_tick = 4;
        g->aud_lowpass = 30;
    } else if (g->events_frame & EVENT_HIT_ENEMY) {
        g->freeze_tick = 2;
    }

    objs_cull_to_delete(g);
    particles_update(g, &g->particles);

    if (g->coins_added) {
        if (g->coins_added_ticks) {
            g->coins_added_ticks--;
        } else {
            i32 to_add = clamp_i(g->coins_added, -2, +2);
            g->save.coins += to_add;
            g->coins_added -= to_add;
        }
    }

    for (obj_each(g, o)) {
        if (o->on_animate) {
            o->on_animate(g, o);
        }
    }
}

void game_resume(game_s *g)
{
    g->item_select.docked = 0;
}

void game_paused(game_s *g)
{
}

i32 gameplay_time(game_s *g)
{
    return g->gameplay_tick;
}

i32 gameplay_time_since(game_s *g, i32 t)
{
    return (g->gameplay_tick - t);
}

bool32 game_load_savefile(game_s *g)
{
    void *f = pltf_file_open_r(SAVEFILE_NAME);

    if (!f) {
        pltf_log("New game\n");
        return 1;
    }

    save_s *hs = &g->save;

    i32    bytes_r = pltf_file_r(f, hs, sizeof(save_s));
    bool32 closed  = pltf_file_close(f);
    bool32 success = (bytes_r == isizeof(save_s) && closed);
    if (!success) {
        pltf_log("+++ Error loading savefile!\n");
        BAD_PATH
        return 0;
    }

    game_load_map(g, hs->hero_mapfile);
    obj_s  *oh   = hero_create(g);
    hero_s *hero = &g->hero_mem;

    oh->pos.x = hs->hero_pos.x - oh->w / 2;
    oh->pos.y = hs->hero_pos.y - oh->h;
    game_prepare_new_map(g);
    return success;
}

bool32 game_save_savefile(game_s *g)
{
    save_s *hs = &g->save;

    void *f = pltf_file_open_w(SAVEFILE_NAME);
    if (!f) {
        pltf_log("+++ Can't write savefile!\n");
        return 0;
    }
    pltf_log("SAVED!\n");
    g->save_ticks  = 1;
    i32    bytes_w = pltf_file_w(f, (const void *)hs, sizeof(save_s));
    bool32 closed  = pltf_file_close(f);
    return (bytes_w == isizeof(save_s) && closed);
}

void game_on_trigger(game_s *g, i32 trigger)
{
    if (!trigger) return;
    for (obj_each(g, o)) {
        if (o->on_trigger) {
            o->on_trigger(g, o, trigger);
        }
    }
}

i32 tick_to_index_freq(i32 tick, i32 n_frames, i32 freqticks)
{
    i32 i = ((tick * n_frames) / freqticks) % n_frames;
    assert(0 <= i && i < n_frames);
    return i;
}

void game_on_solid_appear(game_s *g)
{
    obj_s *ohero = obj_get_tagged(g, OBJ_TAG_HERO);
    if (ohero) {
        hero_check_rope_intact(g, ohero);
    }

    for (obj_each(g, o)) {
        obj_try_wiggle(g, o);
    }
}

void obj_game_player_attackbox_o(game_s *g, obj_s *o, hitbox_s box);

void obj_game_player_attackboxes(game_s *g, hitbox_s *boxes, i32 nb)
{
    for (obj_each(g, o)) {
        rec_i32 aabb = obj_aabb(o);

        i32 strongest = -1;
        for (i32 n = 0; n < nb; n++) {
            hitbox_s hb = boxes[n];
            if (!overlap_rec(aabb, hb.r)) continue;

            if (strongest < 0 || boxes[strongest].damage < hb.damage) {
                strongest = n;
            }
        }

        if (0 <= strongest) {
            obj_game_player_attackbox_o(g, o, boxes[strongest]);
        }
    }
}

void obj_game_player_attackbox(game_s *g, hitbox_s box)
{
    obj_game_player_attackboxes(g, &box, 1);
}

void obj_game_player_attackbox_o(game_s *g, obj_s *o, hitbox_s box)
{
    switch (o->ID) {
    case OBJ_ID_SWITCH: switch_on_interact(g, o); break;
    }

    if ((o->flags & OBJ_FLAG_ENEMY) && !o->enemy.invincible) {
        o->health -= box.damage;
        if (o->health <= 0) {
            obj_delete(g, o);
        } else {
            o->enemy.hurt_tick = ENEMY_HURT_TICKS;
        }
    }
}