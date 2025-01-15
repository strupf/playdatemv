// =============================================================================
// Copyright 2024, Lukas Wolski (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef OBJ_H
#define OBJ_H

#define ENEMY_HURT_TICKS 20

#include "gamedef.h"
#include "objdef.h"
#include "rope.h"

#define NUM_OBJ_POW_2           9 // = 2^N
#define NUM_OBJ                 (1 << NUM_OBJ_POW_2)
#define OBJ_GID_INDEX_SH        (32 - NUM_OBJ_POW_2)
#define OBJ_GID_GEN_MASK        (((u32)1 << OBJ_GID_INDEX_SH) - 1)
#define OBJ_MEM_BYTES           512
#define OBJ_FLAG_MOVER          ((u64)1 << 0)
#define OBJ_FLAG_INTERACTABLE   ((u64)1 << 2)
#define OBJ_FLAG_PLATFORM       ((u64)1 << 3)
#define OBJ_FLAG_HERO_PLATFORM  ((u64)1 << 4) // platform for the hero only
#define OBJ_FLAG_HERO_STOMPABLE ((u64)1 << 5) // platform for the hero only while stomping
#define OBJ_FLAG_HERO_JUMPABLE  ((u64)1 << 6) // platform for the hero only while falling
#define OBJ_FLAG_KILL_OFFSCREEN ((u64)1 << 7)
#define OBJ_FLAG_HOOKABLE       ((u64)1 << 8)
#define OBJ_FLAG_SPRITE         ((u64)1 << 9)
#define OBJ_FLAG_ENEMY          ((u64)1 << 10)
#define OBJ_FLAG_COLLECTIBLE    ((u64)1 << 11)
#define OBJ_FLAG_HURT_ON_TOUCH  ((u64)1 << 12)
#define OBJ_FLAG_CLAMP_ROOM_X   ((u64)1 << 15)
#define OBJ_FLAG_CLAMP_ROOM_Y   ((u64)1 << 16)
#define OBJ_FLAG_BOSS           ((u64)1 << 17)
#define OBJ_FLAG_HOVER_TEXT     ((u64)1 << 18)
#define OBJ_FLAG_CLIMBABLE      ((u64)1 << 19)
#define OBJ_FLAG_LIGHT          ((u64)1 << 20)
#define OBJ_FLAG_ACTOR          ((u64)1 << 61)
#define OBJ_FLAG_SOLID          ((u64)1 << 62)
#define OBJ_FLAG_RENDER_AABB    ((u64)1 << 63)

#define OBJ_FLAG_PLATFORM_ANY (OBJ_FLAG_PLATFORM | OBJ_FLAG_HERO_PLATFORM)

#define OBJ_FLAG_HERO_JUMPSTOMPABLE (OBJ_FLAG_HERO_STOMPABLE | \
                                     OBJ_FLAG_HERO_JUMPABLE)
#define OBJ_FLAG_CLAMP_TO_ROOM (OBJ_FLAG_CLAMP_ROOM_X | OBJ_FLAG_CLAMP_ROOM_Y)

enum obj_bump_flags_e {
    OBJ_BUMP_X_NEG     = 1 << 0,
    OBJ_BUMP_X_POS     = 1 << 1,
    OBJ_BUMP_Y_NEG     = 1 << 2,
    OBJ_BUMP_Y_POS     = 1 << 3,
    OBJ_BUMP_SQUISH    = 1 << 4,
    OBJ_BUMP_ON_HEAD   = 1 << 5,
    OBJ_BUMP_X_BOUNDS  = 1 << 6,
    OBJ_BUMP_Y_BOUNDS  = 1 << 7,
    OBJ_BUMP_JUMPED_ON = 1 << 8,
    OBJ_BUMP_X         = OBJ_BUMP_X_NEG | OBJ_BUMP_X_POS,
    OBJ_BUMP_Y         = OBJ_BUMP_Y_NEG | OBJ_BUMP_Y_POS,
    OBJ_BUMP_XY        = OBJ_BUMP_X | OBJ_BUMP_Y
};

static inline i32 obj_bump_x_flag(i32 x)
{
    if (0 < x) return OBJ_BUMP_X_POS;
    if (x < 0) return OBJ_BUMP_X_NEG;
    return 0;
}

static inline i32 obj_bump_y_flag(i32 y)
{
    if (0 < y) return OBJ_BUMP_Y_POS;
    if (y < 0) return OBJ_BUMP_Y_NEG;
    return 0;
}

enum {
    OBJ_MOVER_GLUE_GROUND        = 1 << 0,
    OBJ_MOVER_ONE_WAY_PLAT       = 1 << 1,
    OBJ_MOVER_SLIDE_Y_POS        = 1 << 2,
    OBJ_MOVER_SLIDE_Y_NEG        = 1 << 3,
    OBJ_MOVER_SLIDE_X_POS        = 1 << 4,
    OBJ_MOVER_SLIDE_X_NEG        = 1 << 5,
    OBJ_MOVER_AVOID_HEADBUMP     = 1 << 7,
    OBJ_MOVER_TERRAIN_COLLISIONS = 1 << 9,
};

#define OBJ_HOVER_TEXT_TICKS 20
typedef void (*obj_action_s)(g_s *g, obj_s *o);

typedef struct {
    texrec_s trec;
    v2_i16   offs;
    i16      flip;
} obj_sprite_s;

typedef struct enemy_s {
    u16   sndID_hurt;
    u16   sndID_die;
    u8    die_tick;
    u8    hurt_tick;
    v2_i8 hurt_shake_offs;
    bool8 invincible;
} enemy_s;

typedef void (*obj_on_update_f)(g_s *g, obj_s *o);
typedef void (*obj_on_animate_f)(g_s *g, obj_s *o);
typedef void (*obj_on_draw_f)(g_s *g, obj_s *o, v2_i32 cam);
typedef void (*obj_on_trigger_f)(g_s *g, obj_s *o, i32 trigger);
typedef void (*obj_on_interact_f)(g_s *g, obj_s *o);

#define OBJ_MAGIC U32_C(0xABABABAB)
struct obj_s {
    obj_s            *next;  // linked list
    u32               GID;   // generational index
    u16               ID;    // type of object
    u16               subID; // subtype of object
    u64               flags;
    u32               tags;
    //
    obj_on_update_f   on_update;
    obj_on_animate_f  on_animate;
    obj_on_draw_f     on_draw;
    obj_on_trigger_f  on_trigger;
    obj_on_interact_f on_interact;
    //
    i32               render_priority;
    u16               bumpflags; // has to be cleared manually
    u16               moverflags;
    i16               w;
    i16               h;
    v2_i32            pos; // position in pixels
    v2_i16            subpos_q8;
    v2_i16            v_q8;
    v2_i16            v_prev_q8;
    // some generic behaviour fields
    i32               trigger;
    i16               state;
    i16               substate;
    i16               action;
    i16               subaction;
    i16               facing; // -1 left, +1 right
    i32               animation;
    i32               timer;
    i32               subtimer;
    u16               cam_attract_r;
    u8                health;
    u8                health_max;
    bool8             interactable_hovered;
    u8                light_radius;
    u8                light_strength;
    enemy_s           enemy;
    //
    ropenode_s       *ropenode;
    rope_s           *rope;
    obj_handle_s      linked_solid;
    //
    i32               n_sprites;
    obj_sprite_s      sprites[4];
    char              filename[64];
    //
    void             *heap;
    byte              mem[OBJ_MEM_BYTES];
    u32               magic;
};

#define obj_get_hero(G) obj_get_tagged(G, OBJ_TAG_HERO)

obj_handle_s obj_handle_from_obj(obj_s *o);
obj_s       *obj_from_obj_handle(obj_handle_s h);
bool32       obj_try_from_obj_handle(obj_handle_s h, obj_s **o_out);
bool32       obj_handle_valid(obj_handle_s h);
obj_s       *obj_create(g_s *g);
void         obj_delete(g_s *g, obj_s *o); // only flags for deletion -> deleted at end of frame
bool32       obj_tag(g_s *g, obj_s *o, i32 tag);
bool32       obj_untag(g_s *g, obj_s *o, i32 tag);
obj_s       *obj_get_tagged(g_s *g, i32 tag);
void         objs_cull_to_delete(g_s *g); // removes all flagged objects
bool32       overlap_obj(obj_s *a, obj_s *b);
rec_i32      obj_aabb(obj_s *o);
rec_i32      obj_rec_left(obj_s *o);
rec_i32      obj_rec_right(obj_s *o);
rec_i32      obj_rec_bottom(obj_s *o);
rec_i32      obj_rec_top(obj_s *o);
v2_i32       obj_pos_bottom_center(obj_s *o);
v2_i32       obj_pos_center(obj_s *o);
void         obj_move(g_s *g, obj_s *o, i32 dx, i32 dy);
bool32       obj_step_is_clamped(g_s *g, obj_s *o, i32 sx, i32 sy);
void         obj_move_by_v_q8(g_s *g, obj_s *o);
void         obj_step_solid(g_s *g, obj_s *o, i32 sx, i32 sy);
bool32       obj_try_wiggle(g_s *g, obj_s *o);
bool32       obj_grounded(g_s *g, obj_s *o);
bool32       obj_grounded_at_offs(g_s *g, obj_s *o, v2_i32 offs);
bool32       obj_would_fall_down_next(g_s *g, obj_s *o, i32 xdir); // not on ground returns false
void         squish_delete(g_s *g, obj_s *o);
void         obj_on_hooked(g_s *g, obj_s *o);
void         obj_move_by_q8(g_s *g, obj_s *o, i32 dx_q8, i32 dy_q8);
void         obj_move_by_v_q8(g_s *g, obj_s *o);
void         obj_v_q8_mul(obj_s *o, i32 mx_q8, i32 my_q8);
void         obj_vx_q8_mul(obj_s *o, i32 mx_q8);
void         obj_vy_q8_mul(obj_s *o, i32 my_q8);
bool32       obj_on_platform(g_s *g, obj_s *o, i32 x, i32 y, i32 w);
enemy_s      enemy_default();

#endif