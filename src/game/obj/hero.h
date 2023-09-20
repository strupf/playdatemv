// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef HERO_H
#define HERO_H

typedef struct hero_s hero_s;

#include "game/obj.h"
#include "game/rope.h"

enum hero_item {
        HERO_ITEM_BOW,
        HERO_ITEM_HOOK,
        HERO_ITEM_SWORD,
        HERO_ITEM_BOMB,
        //
        NUM_HERO_ITEMS,
};

enum hero_state_machine {
        HERO_STATE_LADDER,
        HERO_STATE_GROUND,
        HERO_STATE_AIR,
};

struct hero_s {
        objhandle_s obj;
        int         state;
        //
        bool32      itemselection_dirty;
        i32         jumpticks;
        i32         edgeticks;
        bool32      wasgrounded;
        i32         facingticks; // how long the player is already facing that direction (signed)
        bool32      caninteract;
        i32         swordticks;
        int         sworddir;
        bool32      onladder;
        int         ladderx;
        v2_i32      ppos;
        flags32     aquired_items;
        int         selected_item;
        int         selected_item_prev;
        int         selected_item_next;
        objhandle_s hook;
        rope_s      rope;
        int         pickups;
        char        playername[LEN_STR_PLAYER_NAME];
};

obj_s *hero_create(game_s *g, hero_s *h);
void   hero_update(game_s *g, obj_s *o);
bool32 hero_has_item(hero_s *h, int itemID);
void   hero_set_cur_item(hero_s *h, int itemID);
void   hero_aquire_item(hero_s *h, int itemID);

#endif