// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef HERO_H
#define HERO_H

#include "game/gamedef.h"
#include "game/rope.h"

enum hero_item {
        HERO_ITEM_BOW,
        HERO_ITEM_HOOK,
        HERO_ITEM_SWORD,
        HERO_ITEM_BOMB,
        //
        NUM_HERO_ITEMS,
};

struct hero_s {
        objhandle_s obj;
        i32         jumpticks;
        i32         edgeticks;
        bool32      wasgrounded;
        i32         facingticks; // how long the player is already facing that direction (signed)

        i32 swordticks;
        int sworddir;

        flags32 aquired_items;
        int     selected_item;
        int     selected_item_prev;
        int     selected_item_next;

        objhandle_s hook;
        rope_s      rope;
        int         pickups;
};

obj_s *hero_create(game_s *g, hero_s *h);
void   hero_update(game_s *g, obj_s *o, void *arg);
void   hero_set_cur_item(hero_s *h, int itemID);
void   hero_aquire_item(hero_s *h, int itemID);

#endif