// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#ifndef AUD_H
#define AUD_H

#include "sys/sys.h"

typedef struct {
    sys_wavdata_s wav;
} snd_s;

enum {
    MUS_FADE_NONE,
    MUS_FADE_OUT,
    MUS_FADE_IN,
};

typedef struct {
    int  mus_fade_ticks;
    int  mus_fade_ticks_max;
    int  mus_fade_in;
    int  mus_fade;
    char mus_new[64];
} AUD_s;

extern AUD_s AUD;

void   aud_update();
snd_s  snd_load(const char *filename, void *(*allocf)(usize s));
void   snd_play(snd_s s);
void   snd_play_ext(snd_s s, float vol, float pitch);
void   mus_fade_to(const char *filename, int ticks_out, int ticks_in);
void   mus_stop();
bool32 mus_playing();

#endif