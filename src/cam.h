// =============================================================================
// Copyright 2024, Lukas Wolski (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef CAM_H_
#define CAM_H_

#include "gamedef.h"

enum {
    CAM_MODE_DIRECT,
    CAM_MODE_FOLLOW_HERO,
    CAM_MODE_PIN_TO_TARGET,
};

typedef struct cam_s {
    v2_i32 prev_gfx_offs;
    v2_i32 pos_q8;
    v2_i32 shake;
    v2_i32 attract;
    v2_i32 hero_off;
    i32    lookdown;
    bool8  can_align_x;
    bool8  can_align_y;
    i16    mode;
    u16    shake_ticks;
    u16    shake_ticks_max;
    u16    shake_str_x;
    u16    shake_str_y;
    i16    offs_x;
    bool8  locked_x;
    bool8  locked_y;
} cam_s;

void    cam_screenshake(cam_s *c, i32 ticks, i32 str);
void    cam_screenshake_xy(cam_s *c, i32 ticks, i32 str_x, i32 str_y);
v2_i32  cam_pos_px_top_left(g_s *g, cam_s *c);
v2_i32  cam_pos_px_center(g_s *g, cam_s *c);
rec_i32 cam_rec_px(g_s *g, cam_s *c);
void    cam_init_level(g_s *g, cam_s *c);
void    cam_update(g_s *g, cam_s *c);
v2_i32  cam_offset_max(g_s *g, cam_s *c);
f32     cam_snd_scale(g_s *g, v2_i32 p, u32 dst_max);

#endif