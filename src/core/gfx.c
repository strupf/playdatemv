// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================

#include "gfx.h"
#include "pltf/pltf.h"
#include "spm.h"
#include "util/json.h"
#include "util/mathfunc.h"
#include "util/mem.h"
#include "util/str.h"

fnt_s fnt_load(const char *filename, alloc_s ma)
{
    spm_push();

    fnt_s f = {0};
    char *txt;
    if (!txt_load(filename, spm_alloc, &txt)) {
        pltf_log("+++ error loading font txt\n");
        return f;
    }

    f.widths = (u8 *)ma.allocf(ma.ctx, sizeof(u8) * 256);
    if (!f.widths) {
        pltf_log("+++ allocating font memory\n");
        spm_pop();
        return f;
    }

    json_s j;
    json_root(txt, &j);

    // replace .json with .tex
    char filename_tex[64];
    str_cpy(filename_tex, filename);
    char *fp = &filename_tex[str_len(filename_tex) - 5];
    assert(str_eq(fp, ".json"));
    str_cpy(fp, ".tex");

    pltf_log("  loading fnt tex: %s\n", filename_tex);
    f.t      = tex_load(filename_tex, ma);
    f.grid_w = jsonk_u32(j, "gridwidth");
    f.grid_h = jsonk_u32(j, "gridheight");

    json_key(j, "glyphwidths", &j);
    json_fchild(j, &j);
    for (int i = 0; i < 256; i++) {
        f.widths[i] = json_u32(j);
        json_next(j, &j);
    }

    spm_pop();
    return f;
}

tex_s tex_framebuffer()
{
    tex_s t = {0};
    t.fmt   = TEX_FMT_OPAQUE;
    t.px    = (u32 *)pltf_1bit_buffer();
    t.w     = PLTF_DISPLAY_W;
    t.h     = PLTF_DISPLAY_H;
    t.wword = PLTF_DISPLAY_WWORDS;
    return t;
}

tex_s tex_create_(int w, int h, bool32 mask, alloc_s ma)
{
    tex_s t        = {0};
    i32   waligned = (w + 31) & ~31;
    i32   wword    = (waligned / 32) << (0 < mask);
    usize size     = sizeof(u32) * wword * h;
    void *mem      = ma.allocf(ma.ctx, size); // * 2 bc of mask pixels
    if (!mem) return t;
    t.px    = (u32 *)mem;
    t.fmt   = (0 < mask);
    t.w     = w;
    t.h     = h;
    t.wword = wword;
    return t;
}

tex_s tex_create(int w, int h, alloc_s ma)
{
    return tex_create_(w, h, 1, ma);
}

tex_s tex_create_opaque(int w, int h, alloc_s ma)
{
    return tex_create_(w, h, 0, ma);
}

tex_s tex_load(const char *path, alloc_s ma)
{
    void *f = pltf_file_open(path, PLTF_FILE_R);
    if (!f) {
        pltf_log("Can't load tex: %s\n", path);
        tex_s t0 = {0};
        return t0;
    }

    uint w;
    uint h;
    pltf_file_r(f, &w, sizeof(uint));
    pltf_file_r(f, &h, sizeof(uint));

    usize s = ((w * h) * 2) / 8;
    pltf_log("tex size: %u kb\n", (uint)(s / 1024));
    tex_s t = tex_create(w, h, ma);
    pltf_file_r(f, t.px, s);
    pltf_file_close(f);
    return t;
}

static int tex_px_at_unsafe(tex_s tex, int x, int y)
{
    u32 b = bswap32(0x80000000U >> (x & 31));
    switch (tex.fmt) {
    case TEX_FMT_MASK: return (tex.px[y * tex.wword + ((x >> 5) << 1)] & b);
    case TEX_FMT_OPAQUE: return (tex.px[y * tex.wword + (x >> 5)] & b);
    }
    return 0;
}

static int tex_mk_at_unsafe(tex_s tex, int x, int y)
{
    if (tex.fmt == TEX_FMT_OPAQUE) return 1;

    u32 b = bswap32(0x80000000U >> (x & 31));
    return (tex.px[y * tex.wword + ((x >> 5) << 1) + 1] & b);
}

static void tex_px_unsafe(tex_s tex, int x, int y, int col)
{
    u32  b = bswap32(0x80000000U >> (x & 31));
    u32 *p = NULL;
    switch (tex.fmt) {
    case TEX_FMT_MASK: p = &tex.px[y * tex.wword + ((x >> 5) << 1)]; break;
    case TEX_FMT_OPAQUE: p = &tex.px[y * tex.wword + (x >> 5)]; break;
    default: return;
    }
    *p = (col == 0 ? *p & ~b : *p | b);
}

void tex_px_unsafe_display(tex_s tex, int x, int y, int col)
{
    u32  b = bswap32(0x80000000U >> (x & 31));
    u32 *p = &tex.px[y * tex.wword + (x >> 5)];
    *p     = (col == 0 ? *p & ~b : *p | b);
}

static void tex_mk_unsafe(tex_s tex, int x, int y, int col)
{
    if (tex.fmt == TEX_FMT_OPAQUE) return;
    u32  b = bswap32(0x80000000U >> (x & 31));
    u32 *p = &tex.px[y * tex.wword + ((x >> 5) << 1) + 1];
    *p     = (col == 0 ? *p & ~b : *p | b);
}

int tex_px_at(tex_s tex, int x, int y)
{
    if (x < 0 || tex.w <= x || y < 0 || tex.h <= y) return 0;
    return tex_px_at_unsafe(tex, x, y);
}

int tex_mk_at(tex_s tex, int x, int y)
{
    if (x < 0 || tex.w <= x || y < 0 || tex.h <= y) return 1;
    return tex_mk_at_unsafe(tex, x, y);
}

void tex_px(tex_s tex, int x, int y, int col)
{
    if (0 <= x && x < tex.w && 0 <= y && y < tex.h)
        tex_px_unsafe(tex, x, y, col);
}

void tex_mk(tex_s tex, int x, int y, int col)
{
    if (0 <= x && x < tex.w && 0 <= y && y < tex.h)
        tex_mk_unsafe(tex, x, y, col);
}

void tex_outline(tex_s tex, int x, int y, int w, int h, int col, bool32 dia)
{
    spm_push();
    tex_s src  = tex; // need to work off of a copy
    usize size = sizeof(u32) * tex.wword * tex.h;
    src.px     = (u32 *)spm_alloc(size);
    memcpy(src.px, tex.px, size);

    int x2 = x + w;
    int y2 = y + h;
    for (int yy = y; yy < y2; yy++) {
        for (int xx = x; xx < x2; xx++) {

            if (tex_mk_at_unsafe(src, xx, yy)) continue;

            for (int u = -1; u <= +1; u++) {
                for (int v = -1; v <= +1; v++) {
                    if ((u | v) == 0) continue; // same pixel
                    if (!dia && v != 0 && u != 0) continue;

                    int t = xx + u;
                    int s = yy + v;
                    if (!(x <= t && t < x2 && y <= s && s < y2)) continue;

                    if (tex_mk_at_unsafe(src, t, s)) {
                        tex_mk_unsafe(tex, xx, yy, 1);
                        tex_px_unsafe(tex, xx, yy, col);
                        goto BREAK_LOOP;
                    }
                }
            }
        BREAK_LOOP:;
        }
    }
    spm_pop();
}

gfx_ctx_s gfx_ctx_default(tex_s dst)
{
    gfx_ctx_s ctx = {0};
    ctx.dst       = dst;
    ctx.clip_x2   = dst.w - 1;
    ctx.clip_y2   = dst.h - 1;
    memset(&ctx.pat, 0xFF, sizeof(gfx_pattern_s));
    return ctx;
}

gfx_ctx_s gfx_ctx_display()
{
    return gfx_ctx_default(tex_framebuffer());
}

gfx_ctx_s gfx_ctx_unclip(gfx_ctx_s ctx)
{
    gfx_ctx_s c = ctx;
    c.clip_x1   = 0;
    c.clip_y1   = 0;
    ctx.clip_x2 = ctx.dst.w - 1;
    ctx.clip_y2 = ctx.dst.h - 1;
    return c;
}

gfx_ctx_s gfx_ctx_clip(gfx_ctx_s ctx, i32 x1, i32 y1, i32 x2, i32 y2)
{
    gfx_ctx_s c = ctx;
    c.clip_x1   = max_i(x1, 0);
    c.clip_y1   = max_i(y1, 0);
    c.clip_x2   = min_i(x2, ctx.dst.w - 1);
    c.clip_y2   = min_i(y2, ctx.dst.h - 1);
    return c;
}

gfx_ctx_s gfx_ctx_clip_top(gfx_ctx_s ctx, i32 y1)
{
    gfx_ctx_s c = ctx;
    c.clip_y1   = max_i(y1, 0);
    return c;
}

gfx_ctx_s gfx_ctx_clip_bot(gfx_ctx_s ctx, i32 y2)
{
    gfx_ctx_s c = ctx;
    c.clip_y2   = min_i(y2, ctx.dst.h - 1);
    return c;
}

gfx_ctx_s gfx_ctx_clip_left(gfx_ctx_s ctx, i32 x1)
{
    gfx_ctx_s c = ctx;
    c.clip_x1   = max_i(x1, 0);
    return c;
}

gfx_ctx_s gfx_ctx_clip_right(gfx_ctx_s ctx, i32 x2)
{
    gfx_ctx_s c = ctx;
    c.clip_x2   = min_i(x2, ctx.dst.w - 1);
    return c;
}

gfx_ctx_s gfx_ctx_clipr(gfx_ctx_s ctx, rec_i32 r)
{
    return gfx_ctx_clip(ctx, r.x, r.y, r.x + r.w - 1, r.x + r.h - 1);
}

gfx_ctx_s gfx_ctx_clipwh(gfx_ctx_s ctx, i32 x, i32 y, i32 w, i32 h)
{
    return gfx_ctx_clip(ctx, x, y, x + w - 1, x + h - 1);
}

gfx_pattern_s gfx_pattern_2x2(int p0, int p1)
{
    gfx_pattern_s pat  = {0};
    int           p[2] = {p0, p1};
    for (int i = 0; i < 2; i++) {
        u32 pp       = (u32)p[i];
        u32 pa       = (pp << 6) | (pp << 4) | (pp << 2) | (pp);
        u32 pb       = (pa << 24) | (pa << 16) | (pa << 8) | (pa);
        pat.p[i + 0] = pb;
        pat.p[i + 2] = pb;
        pat.p[i + 4] = pb;
        pat.p[i + 6] = pb;
    }
    return pat;
}

gfx_pattern_s gfx_pattern_4x4(int p0, int p1, int p2, int p3)
{
    gfx_pattern_s pat  = {0};
    u32           p[4] = {p0, p1, p2, p3};
    for (int i = 0; i < 4; i++) {
        u32 pa       = ((u32)p[i] << 4) | ((u32)p[i]);
        u32 pb       = (pa << 24) | (pa << 16) | (pa << 8) | (pa);
        pat.p[i + 0] = pb;
        pat.p[i + 4] = pb;
    }
    return pat;
}

gfx_pattern_s gfx_pattern_8x8(int p0, int p1, int p2, int p3,
                              int p4, int p5, int p6, int p7)
{
    gfx_pattern_s pat  = {0};
    u32           p[8] = {p0, p1, p2, p3, p4, p5, p6, p7};
    for (int i = 0; i < 8; i++) {
        u32 pp   = (u32)p[i];
        pat.p[i] = (pp << 24) | (pp << 16) | (pp << 8) | (pp);
    }
    return pat;
}

gfx_pattern_s gfx_pattern_bayer_4x4(int i)
{
    static const u32 ditherpat[GFX_PATTERN_NUM * 4] = {
        0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U,
        0x88888888U, 0x00000000U, 0x00000000U, 0x00000000U,
        0x88888888U, 0x00000000U, 0x22222222U, 0x00000000U,
        0xAAAAAAAAU, 0x00000000U, 0x22222222U, 0x00000000U,
        0xAAAAAAAAU, 0x00000000U, 0xAAAAAAAAU, 0x00000000U,
        0xAAAAAAAAU, 0x44444444U, 0xAAAAAAAAU, 0x00000000U,
        0xAAAAAAAAU, 0x44444444U, 0xAAAAAAAAU, 0x11111111U,
        0xAAAAAAAAU, 0x55555555U, 0xAAAAAAAAU, 0x11111111U,
        0xAAAAAAAAU, 0x55555555U, 0xAAAAAAAAU, 0x55555555U,
        0xEEEEEEEEU, 0x55555555U, 0xAAAAAAAAU, 0x55555555U,
        0xEEEEEEEEU, 0x55555555U, 0xBBBBBBBBU, 0x55555555U,
        0xFFFFFFFFU, 0x55555555U, 0xBBBBBBBBU, 0x55555555U,
        0xFFFFFFFFU, 0x55555555U, 0xFFFFFFFFU, 0x55555555U,
        0xFFFFFFFFU, 0xDDDDDDDDU, 0xFFFFFFFFU, 0x55555555U,
        0xFFFFFFFFU, 0xDDDDDDDDU, 0xFFFFFFFFU, 0x77777777U,
        0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0x77777777U,
        0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU};

    const u32    *p   = &ditherpat[clamp_i(i, 0, GFX_PATTERN_MAX) << 2];
    gfx_pattern_s pat = {{p[0], p[1], p[2], p[3], p[0], p[1], p[2], p[3]}};
    return pat;
}

gfx_pattern_s gfx_pattern_interpolate(int num, int den)
{
    return gfx_pattern_bayer_4x4((num * GFX_PATTERN_NUM) / den);
}

gfx_pattern_s gfx_pattern_interpolatec(int num, int den,
                                       int (*ease)(int a, int b, int num, int den))
{
    int i = ease(0, 16, num, den);
    return gfx_pattern_bayer_4x4(i);
}

void tex_clr(tex_s dst, int col)
{
    i32  N = dst.wword * dst.h;
    u32 *p = dst.px;
    if (!p) return;
    switch (col) {
    case GFX_COL_BLACK:
        switch (dst.fmt) {
        case TEX_FMT_OPAQUE:
            for (i32 n = 0; n < N; n++) {
                *p++ = 0U;
            }
            break;
        case TEX_FMT_MASK:
            for (i32 n = 0; n < N; n += 2) {
                *p++ = 0U;          // data
                *p++ = 0xFFFFFFFFU; // mask
            }
            break;
        }
        break;
    case GFX_COL_WHITE:
        switch (dst.fmt) {
        case TEX_FMT_OPAQUE:
            for (i32 n = 0; n < N; n++) {
                *p++ = 0xFFFFFFFFU;
            }
            break;
        case TEX_FMT_MASK:
            for (i32 n = 0; n < N; n += 2) {
                *p++ = 0xFFFFFFFFU; // data
                *p++ = 0xFFFFFFFFU; // mask
            }
            break;
        }
        break;
    case GFX_COL_CLEAR:
        if (dst.fmt == TEX_FMT_OPAQUE) break;
        for (i32 n = 0; n < N; n++) {
            *p++ = 0;
        }
        break;
    }
}

// used for simple sprites
typedef struct {
    i32           dmax; // count of dst words -1
    u32           ml;   // boundary mask left
    u32           mr;   // boundary mask right
    i32           mode; // drawing mode
    i32           doff; // bitoffset of first dst bit
    u32          *dp;   // pixel
    i32           y;
    gfx_pattern_s pat;
    i32           dadd;
} span_blit_s;

span_blit_s span_blit_gen(gfx_ctx_s ctx, int y, int x1, int x2, int mode)
{
    i32         nbit = (x2 + 1) - x1; // number of bits in a row to blit
    i32         lsh  = (ctx.dst.fmt == TEX_FMT_MASK);
    span_blit_s info = {0};
    info.y           = y;
    info.doff        = x1 & 31;
    info.dmax        = (info.doff + nbit - 1) >> 5;                        // number of touched dst words -1
    info.mode        = mode;                                               // sprite masking mode
    info.ml          = bswap32(0xFFFFFFFFU >> (31 & info.doff));           // mask to cut off boundary left
    info.mr          = bswap32(0xFFFFFFFFU << (31 & (-info.doff - nbit))); // mask to cut off boundary right
    info.dp          = &ctx.dst.px[((x1 >> 5) << lsh) + y * ctx.dst.wword];
    info.dadd        = 1 + lsh;
    info.pat         = ctx.pat;
    return info;
}

void gfx_spr_tiled(gfx_ctx_s ctx, texrec_s src, v2_i32 pos, int flip, int mode, int tx, int ty)
{
    if ((src.r.w | src.r.h) == 0) return;
    int dx = tx ? tx : src.r.w;
    int dy = ty ? ty : src.r.h;
    int x1 = tx ? ((pos.x % dx) - dx) % dx : pos.x;
    int y1 = ty ? ((pos.y % dy) - dy) % dy : pos.y;
    int x2 = tx ? ((ctx.dst.w - x1) / dx) * dx : x1;
    int y2 = ty ? ((ctx.dst.h - y1) / dy) * dy : y1;

    for (int y = y1; y <= y2; y += dy) {
        for (int x = x1; x <= x2; x += dx) {
            v2_i32 p = {x, y};
            gfx_spr(ctx, src, p, flip, mode);
        }
    }
}

void gfx_spr_tileds(gfx_ctx_s ctx, texrec_s src, v2_i32 pos, int flip, int mode, bool32 x, bool32 y)
{
    gfx_spr_tiled(ctx, src, pos, flip, mode, x ? src.r.w : 0, y ? src.r.h : 0);
}

void gfx_spr_rotscl(gfx_ctx_s ctx, texrec_s src, v2_i32 pos, v2_i32 origin, f32 angle,
                    f32 sclx, f32 scly)
{
    tex_s dst = ctx.dst;
    origin.x += src.r.x;
    origin.y += src.r.y;
    m33_f32 m1 = m33_offset(+(f32)origin.x, +(f32)origin.y);
    m33_f32 m3 = m33_offset(-(f32)pos.x - (f32)origin.x, -(f32)pos.y - (f32)origin.y);
    m33_f32 m4 = m33_rotate(angle);
    m33_f32 m5 = m33_scale(sclx, scly);

    m33_f32 m = m33_identity();
    m         = m33_mul(m, m3);
    m         = m33_mul(m, m5);
    m         = m33_mul(m, m4);
    m         = m33_mul(m, m1);

    int x1 = 0; // optimize by inverting the matrix and calc corners
    int y1 = 0;
    int x2 = ctx.dst.w - 1;
    int y2 = ctx.dst.h - 1;
    int w1 = x1 >> 5;
    int w2 = x2 >> 5;

    f32 u1 = (f32)(src.r.x);
    f32 v1 = (f32)(src.r.y);
    f32 u2 = (f32)(src.r.x + src.r.w);
    f32 v2 = (f32)(src.r.y + src.r.h);

    for (int y = y1; y <= y2; y++) {
        u32 pat = ctx.pat.p[y & 7];
        for (int wi = w1; wi <= w2; wi++) {

            int p1 = wi == w1 ? x1 & 31 : 0;
            int p2 = wi == w2 ? x2 & 31 : 31;
            u32 sp = 0;
            u32 sm = 0;

            for (int p = p1; p <= p2; p++) {
                int x = (wi << 5) + p;

                f32 t = (f32)(x + src.r.x);
                f32 s = (f32)(y + src.r.y);
                int u = (int)(t * m.m[0] + s * m.m[1] + m.m[2] + .5f);
                int v = (int)(t * m.m[3] + s * m.m[4] + m.m[5] + .5f);
                if (!(u1 <= u && u < u2)) continue;
                if (!(v1 <= v && v < v2)) continue;
                u32 bit = 0x80000000U >> p;

                if (tex_px_at_unsafe(src.t, u, v)) sp |= bit;
                if (tex_mk_at_unsafe(src.t, u, v)) sm |= bit;
            }

            u32 *dp = NULL;
            u32 *dm = NULL;
            switch (ctx.dst.fmt) {
            case TEX_FMT_OPAQUE:
                dp = &ctx.dst.px[y * dst.wword + wi];
                break;
            case TEX_FMT_MASK:
                dp = &dst.px[y * dst.wword + (wi << 1)];
                dm = dp + 1;
                break;
            default: BAD_PATH
            }

            spr_blit(dp, dm, bswap32(sp), bswap32(sm) & pat, 0);
        }
    }
}

void gfx_spr_rotated(gfx_ctx_s ctx, texrec_s src, v2_i32 pos, v2_i32 origin, f32 angle)
{
    gfx_spr_rotscl(ctx, src, pos, origin, angle, 1.f, 1.f);
}

static void apply_prim_mode(u32 *restrict dp, u32 *restrict dm, u32 sm, int mode, u32 pt)
{
    switch (mode) {
    case PRIM_MODE_INV: sm &= pt, *dp = (*dp & ~sm) | (~*dp & sm); break;
    case PRIM_MODE_WHITE: sm &= pt, *dp |= sm; break;
    case PRIM_MODE_BLACK: sm &= pt, *dp &= ~sm; break;
    case PRIM_MODE_WHITE_BLACK: pt = ~pt; // fallthrough
    case PRIM_MODE_BLACK_WHITE: *dp = (*dp & ~(sm & pt)) | (sm & ~pt); break;
    }

    if (dm) *dm |= sm;
}

static void prim_blit_span(span_blit_s info)
{
    u32 *restrict dp = (u32 *restrict)info.dp;
    u32 pt           = info.pat.p[info.y & 7];
    u32 m            = info.ml;
    for (int i = 0; i < info.dmax; i++) {
        apply_prim_mode(dp, info.dadd == 2 ? dp + 1 : NULL, m, info.mode, pt);
        m = 0xFFFFFFFFU;
        dp += info.dadd;
    }
    apply_prim_mode(dp, info.dadd == 2 ? dp + 1 : NULL, m & info.mr, info.mode, pt);
}

static void apply_prim_mode_X(u32 *restrict dp, u32 sm, int mode, u32 pt)
{
    switch (mode) {
    case PRIM_MODE_INV: sm &= pt, *dp = (*dp & ~sm) | (~*dp & sm); break;
    case PRIM_MODE_WHITE: sm &= pt, *dp |= sm; break;
    case PRIM_MODE_BLACK: sm &= pt, *dp &= ~sm; break;
    case PRIM_MODE_WHITE_BLACK: pt = ~pt; // fallthrough
    case PRIM_MODE_BLACK_WHITE: *dp = (*dp & ~(sm & pt)) | (sm & ~pt); break;
    }
}

static void prim_blit_span_X(span_blit_s info)
{
    u32 *restrict dp = (u32 *restrict)info.dp;
    u32 pt           = info.pat.p[info.y & 7];
    u32 m            = info.ml;
    for (int i = 0; i < info.dmax; i++) {
        apply_prim_mode_X(dp, m, info.mode, pt);
        m = 0xFFFFFFFFU;
        dp++;
    }
    apply_prim_mode_X(dp, m & info.mr, info.mode, pt);
}

static void prim_blit_span_Y(span_blit_s info)
{
    u32 *restrict dp = (u32 *restrict)info.dp;
    u32 pt           = info.pat.p[info.y & 7];
    u32 m            = info.ml;
    for (int i = 0; i < info.dmax; i++) {
        apply_prim_mode(dp, dp + 1, m, info.mode, pt);
        m = 0xFFFFFFFFU;
        dp += 2;
    }
    apply_prim_mode(dp, dp + 1, m & info.mr, info.mode, pt);
}

void gfx_rec_fill(gfx_ctx_s ctx, rec_i32 rec, int mode)
{
    int x1 = max_i(rec.x, ctx.clip_x1); // area bounds on canvas [x1/y1, x2/y2]
    int y1 = max_i(rec.y, ctx.clip_y1);
    int x2 = min_i(rec.x + rec.w - 1, ctx.clip_x2);
    int y2 = min_i(rec.y + rec.h - 1, ctx.clip_y2);
    if (x2 < x1) return;

    tex_s       dtex = ctx.dst;
    span_blit_s info = span_blit_gen(ctx, y1, x1, x2, mode);
    if (dtex.fmt == TEX_FMT_OPAQUE) {
        for (info.y = y1; info.y <= y2; info.y++) {
            prim_blit_span_X(info);
            info.dp += dtex.wword;
        }
    } else {
        for (info.y = y1; info.y <= y2; info.y++) {
            prim_blit_span_Y(info);
            info.dp += dtex.wword;
        }
    }
}

void gfx_fill_rows(tex_s dst, gfx_pattern_s pat, int y1, int y2)
{
    assert(0 <= y1 && y2 <= dst.h);
    u32 *px = &dst.px[y1 * dst.wword];
    for (int y = y1; y < y2; y++) {
        const u32 p = pat.p[y & 7];
        for (int x = 0; x < dst.wword; x++) {
            *px++ = p;
        }
    }
}

void fnt_draw_str(gfx_ctx_s ctx, fnt_s fnt, v2_i32 pos, fntstr_s str, int mode)
{
    v2_i32   p = pos;
    texrec_s t;
    t.t   = fnt.t;
    t.r.w = fnt.grid_w;
    t.r.h = fnt.grid_h;
    for (int n = 0; n < str.n; n++) {
        int ci = str.buf[n];
        t.r.x  = (ci & 31) * fnt.grid_w;
        t.r.y  = (ci >> 5) * fnt.grid_h;
        gfx_spr(ctx, t, p, 0, mode);
        p.x += fnt.widths[ci];
    }
}

void fnt_draw_ascii(gfx_ctx_s ctx, fnt_s fnt, v2_i32 pos, const char *text, int mode)
{
    fntstr_s str      = {0};
    u8       buf[256] = {0};
    str.buf           = buf;
    str.cap           = ARRLEN(buf);
    for (const char *c = text; *c != '\0'; c++) {
        if (str.n == str.cap) break;
        str.buf[str.n++] = (u8)*c;
    }
    fnt_draw_str(ctx, fnt, pos, str, mode);
}

void fnt_draw_str_mono(gfx_ctx_s ctx, fnt_s fnt, v2_i32 pos, fntstr_s str, int mode, int spacing)
{
    v2_i32   p = pos;
    texrec_s t;
    t.t   = fnt.t;
    t.r.w = fnt.grid_w;
    t.r.h = fnt.grid_h;
    int s = spacing ? spacing : fnt.grid_w;
    for (int n = 0; n < str.n; n++) {
        int ci = str.buf[n];
        t.r.x  = (ci & 31) * fnt.grid_w;
        t.r.y  = (ci >> 5) * fnt.grid_h;
        gfx_spr(ctx, t, p, 0, mode);
        p.x += s;
    }
}

void fnt_draw_ascii_mono(gfx_ctx_s ctx, fnt_s fnt, v2_i32 pos, const char *text, int mode, int spacing)
{
    fntstr_s str      = {0};
    u8       buf[256] = {0};
    str.buf           = buf;
    str.cap           = ARRLEN(buf);
    for (const char *c = text; *c != '\0'; c++) {
        if (str.n == str.cap) break;
        str.buf[str.n++] = (u8)*c;
    }
    fnt_draw_str_mono(ctx, fnt, pos, str, mode, spacing);
}

int fnt_length_px(fnt_s fnt, const char *txt)
{
    int l = 0;
    for (const char *c = txt; *c != '\0'; c++) {
        l += fnt.widths[(uint)*c];
    }
    return l;
}

int fnt_length_px_mono(fnt_s fnt, const char *txt, int spacing)
{
    int l = 0;
    int s = spacing ? spacing : fnt.grid_w;
    for (const char *c = txt; *c != '\0'; c++) {
        l += s;
    }
    return l;
}

void gfx_tri_fill(gfx_ctx_s ctx, tri_i32 t, int mode)
{
    v2_i32 t0 = t.p[0];
    v2_i32 t1 = t.p[1];
    v2_i32 t2 = t.p[2];
    if (t0.y > t1.y) SWAP(v2_i32, t0, t1);
    if (t0.y > t2.y) SWAP(v2_i32, t0, t2);
    if (t1.y > t2.y) SWAP(v2_i32, t1, t2);
    int th = t2.y - t0.y;
    if (th == 0) return;
    int h1  = t1.y - t0.y + 1;
    int h2  = t2.y - t1.y + 1;
    i32 d0  = t2.x - t0.x;
    i32 d1  = t1.x - t0.x;
    i32 d2  = t2.x - t1.x;
    i32 ya0 = max_i32(ctx.clip_y1, t0.y);
    i32 ya1 = min_i32(ctx.clip_y2, t1.y);

    for (int y = ya0; y <= ya1; y++) {
        assert(ctx.clip_y1 <= y && y <= ctx.clip_y2);
        i32 x1 = t0.x + (d0 * (y - t0.y)) / th;
        i32 x2 = t0.x + (d1 * (y - t0.y)) / h1;
        if (x2 < x1) SWAP(i32, x1, x2);
        x1 = max_i32(x1, ctx.clip_x1);
        x2 = min_i32(x2, ctx.clip_x2);
        if (x2 < x1) continue;
        span_blit_s info = span_blit_gen(ctx, y, x1, x2, mode);
        prim_blit_span(info);
    }

    i32 yb0 = max_i32(ctx.clip_y1, t1.y);
    i32 yb1 = min_i32(ctx.clip_y2, t2.y);

    for (int y = yb0; y <= yb1; y++) {
        assert(ctx.clip_y1 <= y && y <= ctx.clip_y2);
        i32 x1 = t0.x + (d0 * (y - t0.y)) / th;
        i32 x2 = t1.x + (d2 * (y - t1.y)) / h2;
        if (x2 < x1) SWAP(i32, x1, x2);
        x1 = max_i32(x1, ctx.clip_x1);
        x2 = min_i32(x2, ctx.clip_x2);
        if (x2 < x1) continue;
        span_blit_s info = span_blit_gen(ctx, y, x1, x2, mode);
        prim_blit_span(info);
    }
}

// triangle pixels in Q2 - subpixel accuracy!
// naive per pixel loop from my own software renderer
// whole spans for playdate
void gfx_tri_fill_uvw(gfx_ctx_s ctx, v2_i32 tri[3], int mode)
{
    v2_i32 vv[3] = {0};
    for (i32 n = 0; n < 3; n++) {
        vv[n].x = tri[n].x;
        vv[n].y = tri[n].y;
    }

    v2_i32 v12 = v2_sub(vv[2], vv[1]);
    v2_i32 v20 = v2_sub(vv[0], vv[2]);
    v2_i32 v01 = v2_sub(vv[1], vv[0]);
    i32    den = v2_crs(v01, v20); // den is max: (w * subpx) * (h * subpx)

    if (den == 0) return; // invalid triangle
    if (den < 0) {
        den = -den;
        v12 = v2_inv(v12);
        v20 = v2_inv(v20);
        v01 = v2_inv(v01);
    }

    i32 xa = max_i32(min3_i32(v20.x, v12.x, v01.x) >> 2, 0);           // screen x1
    i32 ya = max_i32(min3_i32(v20.y, v12.y, v01.y) >> 2, 0);           // screen y1
    i32 xb = min_i32(max3_i32(v20.x, v12.x, v01.x) >> 2, ctx.clip_x2); // screen x2
    i32 yb = min_i32(max3_i32(v20.y, v12.y, v01.y) >> 2, ctx.clip_y2); // screen y2
    i32 xm = (xa << 2) + 2;                                            // mid point top left
    i32 ym = (ya << 2) + 2;
    i32 u0 = v12.y * (xm - vv[1].x) - v12.x * (ym - vv[1].y); // midpoint for subpixel sampling
    i32 v0 = v20.y * (xm - vv[2].x) - v20.x * (ym - vv[2].y);
    i32 w0 = v01.y * (xm - vv[0].x) - v01.x * (ym - vv[0].y);
    i32 ux = v12.y << 2;
    i32 uy = v12.x << 2;
    i32 vx = v20.y << 2;
    i32 vy = v20.x << 2;
    i32 wx = v01.y << 2;
    i32 wy = v01.x << 2;

    for (i32 y = ya; y <= yb; y++) {
        i32 u  = u0;
        i32 v  = v0;
        i32 w  = w0;
        i32 x1 = 0xFFFF;
        i32 x2 = 0;
        for (i32 x = xa; x <= xb; x++) {
            if (0 <= (u | v | w)) {
                x1 = min_i32(x1, x);
                x2 = max_i32(x2, x);
            }
            u += ux;
            v += vx;
            w += wx;
        }

        span_blit_s i = span_blit_gen(ctx, y, x1, x2, mode);
        prim_blit_span(i);
        u0 -= uy;
        v0 -= vy;
        w0 -= wy;
    }
}

void gfx_cir_fill(gfx_ctx_s ctx, v2_i32 p, int d, int mode)
{
    if (d <= 0) return;
    if (d == 1) {
        gfx_rec_fill(ctx, (rec_i32){p.x, p.y, 1, 1}, mode);
        return;
    }
    if (d == 2) {
        gfx_rec_fill(ctx, (rec_i32){p.x - 1, p.y - 1, 2, 2}, mode);
        return;
    }

    // Jesko's Method, shameless copy
    // https://schwarzers.com/algorithms/
    i32 r = d >> 1;
    i32 x = r;
    i32 y = 0;
    i32 t = r >> 4;

    do {
        i32 x1 = max_i32(p.x - x, ctx.clip_x1);
        i32 x2 = min_i32(p.x + x, ctx.clip_x2);
        i32 x3 = max_i32(p.x - y, ctx.clip_x1);
        i32 x4 = min_i32(p.x + y, ctx.clip_x2);
        i32 y4 = p.y - x; // ordered in y
        i32 y2 = p.y - y;
        i32 y1 = p.y + y;
        i32 y3 = p.y + x;

        if (ctx.clip_y1 <= y4 && y4 <= ctx.clip_y2 && x3 <= x4) {
            prim_blit_span(span_blit_gen(ctx, y4, x3, x4, mode));
        }
        if (ctx.clip_y1 <= y2 && y2 <= ctx.clip_y2 && x1 <= x2) {
            prim_blit_span(span_blit_gen(ctx, y2, x1, x2, mode));
        }
        if (ctx.clip_y1 <= y1 && y1 <= ctx.clip_y2 && x1 <= x2 && y != 0) {
            prim_blit_span(span_blit_gen(ctx, y1, x1, x2, mode));
        }
        if (ctx.clip_y1 <= y3 && y3 <= ctx.clip_y2 && x3 <= x4) {
            prim_blit_span(span_blit_gen(ctx, y3, x3, x4, mode));
        }

        y++;
        t += y;
        i32 k = t - x;
        if (0 <= k) {
            t = k;
            x--;
        }
    } while (y <= x);
}

void gfx_lin(gfx_ctx_s ctx, v2_i32 a, v2_i32 b, int mode)
{
    gfx_lin_thick(ctx, a, b, mode, 1);
}

void gfx_lin_thick(gfx_ctx_s ctx, v2_i32 a, v2_i32 b, int mode, int d)
{
    i32    dx = +abs_i32(b.x - a.x);
    i32    dy = -abs_i32(b.y - a.y);
    i32    sx = a.x < b.x ? +1 : -1;
    i32    sy = a.y < b.y ? +1 : -1;
    i32    er = dx + dy;
    v2_i32 pi = a;

    while (1) {
        gfx_cir_fill(ctx, pi, d, mode);

        if (v2_eq(pi, b)) break;
        i32 e2 = er << 1;
        if (e2 >= dy) { er += dy, pi.x += sx; }
        if (e2 <= dx) { er += dx, pi.y += sy; }
    }
}

void gfx_rec(gfx_ctx_s ctx, rec_i32 r, int mode)
{
    NOT_IMPLEMENTED
}

void gfx_tri(gfx_ctx_s ctx, tri_i32 t, int mode)
{
    gfx_lin_thick(ctx, t.p[0], t.p[1], mode, 2);
    gfx_lin_thick(ctx, t.p[0], t.p[2], mode, 2);
    gfx_lin_thick(ctx, t.p[1], t.p[2], mode, 2);
}

void gfx_cir(gfx_ctx_s ctx, v2_i32 p, int r, int mode)
{
    NOT_IMPLEMENTED
}

int cmp_int_poly(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    if (x < y) return -1;
    if (x > y) return +1;
    return 0;
}

void gfx_poly_fill(gfx_ctx_s ctx, v2_i32 *pt, int n_pt, int mode)
{
    int y1 = 0x10000;
    int y2 = 0;
    for (int i = 0; i < n_pt; i++) {
        y1 = min_i(y1, pt[i].y);
        y2 = max_i(y2, pt[i].y);
    }

    y1 = max_i(y1, ctx.clip_y1);
    y2 = min_i(y2, ctx.clip_y2);

    int nx[64] = {0};
    for (int y = y1; y <= y2; y++) {
        int ns = 0;
        for (int i = 0, j = n_pt - 1; i < n_pt; j = i, i++) {
            v2_i32 pi = pt[i];
            v2_i32 pj = pt[j];
            if (!((pi.y < y && y <= pj.y) || (pj.y < y && y <= pi.y)))
                continue;
            nx[ns++] = pi.x + ((pj.x - pi.x) * (y - pi.y)) / (pj.y - pi.y);
        }

        sort_array(nx, ns, sizeof(int), cmp_int_poly);

        for (int i = 0; i < ns; i += 2) {
            int x1 = nx[i];
            if (ctx.clip_x2 < x1) break;
            int x2 = nx[i + 1];
            if (x2 <= ctx.clip_x1) continue;

            x1 = max_i(x1, ctx.clip_x1);
            x2 = min_i(x2, ctx.clip_x2);

            span_blit_s s = span_blit_gen(ctx, y, x1, x2, mode);
            prim_blit_span(s);
        }
    }
}

void gfx_spr(gfx_ctx_s ctx, texrec_s src, v2_i32 pos, i32 flip, i32 mode)
{
    // area bounds on canvas [x1/y1, x2/y2)
    i32 x1 = max_i32(pos.x, ctx.clip_x1);               // inclusive
    i32 y1 = max_i32(pos.y, ctx.clip_y1);               // inclusive
    i32 x2 = min_i32(pos.x + src.r.w - 1, ctx.clip_x2); // inclusive
    i32 y2 = min_i32(pos.y + src.r.h - 1, ctx.clip_y2); // inclusive
    if (x2 < x1) return;

    tex_s  dtex    = ctx.dst;
    tex_s  stex    = src.t;
    bool32 fx      = (flip & SPR_FLIP_X) != 0;                              // flipped x
    bool32 fy      = (flip & SPR_FLIP_Y) != 0;                              // flipped y
    i32    sy      = fy ? -1 : +1;                                          // sign flip x
    i32    sx      = fx ? -1 : +1;                                          // sign flip y
    i32    nb      = (x2 + 1) - x1;                                         // number of bits in a row
    i32    od      = x1 & 31;                                               // bitoffset in dst
    i32    dm      = (od + nb - 1) >> 5;                                    // number of touched dst words -1
    u32    ml      = bswap32(0xFFFFFFFFU >> (31 & od));                     // mask to cut off boundary left
    u32    mr      = bswap32(0xFFFFFFFFU << (31 & (u32)(-od - nb)));        // mask to cut off boundary right
    i32    u1      = src.r.x - sx * pos.x + (fx ? src.r.w - (x2 + 1) : x1); // first bit index in src row
    i32    os      = (u32)(sx * u1 - fx * nb) & 31;                         // bitoffset in src
    i32    da      = 1 + (dtex.fmt == TEX_FMT_MASK);                        // number of words to next logical pixel word in dst
    i32    sa      = 1 + (stex.fmt == TEX_FMT_MASK);                        // number of words to next logical pixel word in src
    i32    sm      = ((os + nb - 1) >> 5) * sa;                             // number of touched src words -1
    i32    of      = os - od;                                               // alignment difference
    i32    l       = of & 31;                                               // word left shift amount
    i32    r       = 32 - l;                                                // word rght shift amound
    i32    src_wy1 = src.r.y + sy * (y1 - pos.y) + fy * (src.r.h - 1);
    u32   *dst_p   = &dtex.px[(x1 >> 5) * da + dtex.wword * y1];      // dst pixel words
    u32   *src_p   = &stex.px[(u1 >> 5) * sa + stex.wword * src_wy1]; // src pixel words
    i32    incr_sp = stex.wword * sy;
    i32    incr_dp = dtex.wword;

    for (i32 y = y1; y <= y2; y++, src_p += incr_sp, dst_p += incr_dp) {
        u32 pt = ctx.pat.p[y & 7];
        if (pt == 0) continue;

        u32 *restrict dp        = dst_p;
        const u32 *restrict sp1 = src_p;
        const u32 *restrict sp2 = src_p + sm;
        u32 p                   = 0;
        u32 m                   = 0;

        if (fx) {
            const u32 *restrict sp = sp2;
            if (l == 0) { // same alignment, fast path
                p = brev32(*sp);
                m = ml;
                if (sa == 2) {
                    m &= brev32(*(sp + 1));
                }
                sp -= sa;
                for (i32 i = 0; i < dm; i++) {
                    spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & pt, mode);
                    p = brev32(*sp);
                    m = 0xFFFFFFFFU;
                    if (sa == 2) {
                        m = brev32(*(sp + 1));
                    }
                    sp -= sa;
                    dp += da;
                }
                spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & (pt & mr), mode);
                continue;
            }
            if (0 < of) { // first word
                p = brev32(bswap32(*sp)) << l;
                m = 0xFFFFFFFFU;
                if (sa == 2) {
                    m = brev32(bswap32(*(sp + 1))) << l;
                }
                if (0 < sm) {
                    sp -= sa;
                }
            }

            p = bswap32(p | (brev32(bswap32(*sp)) >> r));
            if (sa == 2) {
                m = bswap32(m | (brev32(bswap32(*(sp + 1))) >> r)) & ml;
            } else {
                m = ml;
            }
            if (dm == 0) {
                spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & (pt & mr), mode);
                continue; // only one word long
            }

            spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & pt, mode);
            dp += da;

            for (i32 i = 1; i < dm; i++) { // middle words
                p = brev32(bswap32(*sp)) << l;
                p = bswap32(p | (brev32(bswap32(*(sp - sa))) >> r));
                m = 0xFFFFFFFFU;
                if (sa == 2) {
                    m = brev32(bswap32(*(sp + 1))) << l;
                    m = bswap32(m | (brev32(bswap32(*(sp - 1))) >> r));
                }
                spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & pt, mode);
                sp -= sa;
                dp += da;
            }

            p = brev32(bswap32(*sp)) << l; // last word
            m = 0xFFFFFFFFU;
            if (sa == 2) {
                m = brev32(bswap32(*(sp + 1))) << l;
            }
            if (sp > sp1) {
                sp -= sa;
                p |= brev32(bswap32(*sp)) >> r;
                if (sa == 2) {
                    m |= brev32(bswap32(*(sp + 1))) >> r;
                }
            }
            p = bswap32(p);
            m = bswap32(m);
            spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & (pt & mr), mode);
        } else {
            const u32 *restrict sp = sp1;
            if (l == 0) { // same alignment, fast path
                p = *sp;
                m = ml;
                if (sa == 2) {
                    m &= *(sp + 1);
                }
                sp += sa;
                for (i32 i = 0; i < dm; i++) {
                    spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & pt, mode);
                    p = *sp;
                    m = 0xFFFFFFFFU;
                    if (sa == 2) {
                        m = *(sp + 1);
                    }
                    sp += sa;
                    dp += da;
                }
                spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & (pt & mr), mode);
                continue;
            }

            if (0 < of) { // first word
                p = bswap32(*sp) << l;
                m = 0xFFFFFFFFU;
                if (sa == 2) {
                    m = bswap32(*(sp + 1)) << l;
                }
                if (0 < sm) {
                    sp += sa;
                }
            }

            p = bswap32(p | (bswap32(*sp) >> r));
            if (sa == 2) {
                m = bswap32(m | (bswap32(*(sp + 1)) >> r)) & ml;
            } else {
                m = ml;
            }
            if (dm == 0) {
                spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & (pt & mr), mode);
                continue; // only one word long
            }

            spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & pt, mode);
            dp += da;

            for (i32 i = 1; i < dm; i++) { // middle words
                p = bswap32(*sp) << l;
                p = bswap32(p | (bswap32(*(sp + sa)) >> r));
                m = 0xFFFFFFFFU;
                if (sa == 2) {
                    m = bswap32(*(sp + 1)) << l;
                    m = bswap32(m | (bswap32(*(sp + 3)) >> r));
                }
                spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & pt, mode);
                sp += sa;
                dp += da;
            }

            p = bswap32(*sp) << l; // last word
            m = 0xFFFFFFFFU;
            if (sa == 2) {
                m = bswap32(*(sp + 1)) << l;
            }
            if (sp < sp2) { // this is different for reversed blitting!
                sp += sa;
                p |= bswap32(*sp) >> r;
                if (sa == 2) {
                    m |= bswap32(*(sp + 1)) >> r;
                }
            }
            p = bswap32(p);
            m = bswap32(m);
            spr_blit(dp, da == 1 ? NULL : dp + 1, p, m & (pt & mr), mode);
        }
    }
}

// special case: drawing tiles equal or smaller than 32x32 to an opaque target texture
// no XY flipping supported
void gfx_spr_tile(gfx_ctx_s ctx, tex_s stex, i32 tx, i32 ty, i32 ts, v2_i32 pos)
{
    assert(ctx.dst.fmt == TEX_FMT_OPAQUE);
    assert(stex.fmt == TEX_FMT_MASK);
    assert(ts <= 5);
    gfx_spr(ctx, (texrec_s){stex, {tx << ts, ty << ts, 1 << ts, 1 << ts}}, pos, 0, 0);
}