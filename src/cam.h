// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef CAM_H_
#define CAM_H_

#include "gamedef.h"

#define CAM_ATTRACTOR_RADIUS   300.f
#define CAM_ATTRACTOR_MAX_OFFS 40.f

enum {
    CAM_MODE_DIRECT,
    CAM_MODE_FOLLOW_HERO,
    CAM_MODE_PIN_TO_TARGET,
};

typedef struct {
    v2_f32 pos;
    v2_f32 offs_shake;
    i16    mode;
    i16    look_tick;
    i16    shake_ticks;
    i16    shake_ticks_max;
    i16    shake_str;
    bool8  locked_x;
    bool8  locked_y;
    v2_f32 look_ahead;
    v2_f32 camattr;
    i32    n_attractors;
    v2_i32 attractors[16];
} cam_s;

void    cam_screenshake(cam_s *c, i32 ticks, i32 str);
v2_i32  cam_pos_px(game_s *g, cam_s *c);
rec_i32 cam_rec_px(game_s *g, cam_s *c);
void    cam_set_pos_px(cam_s *c, i32 x, i32 y);
void    cam_init_level(game_s *g, cam_s *c);
void    cam_update(game_s *g, cam_s *c);
v2_i32  cam_offset_max(game_s *g, cam_s *c);

#endif