// =============================================================================
// Copyright (C) 2023, Strupf (the.strupf@proton.me). All rights reserved.
// =============================================================================
/*
 * Tile collisions are based on pixels. Tiles are 16x16 and each tile shape
 * has a corresponding bitmask of solid(1)/empty(0) bits int[16].
 *
 * When checking if a solid pixel is overlapped we loop through the
 * tiles and determine the range in the corresponding bitmasks
 * to check against.
 */

#include "collision.h"

/* contains triangles for each tile describing its shape.
 */
const tri_i32 tilecolliders[] = {
    // dummy triangles
    0, 0, 0, 0, 0, 0, // empty
    0, 0, 0, 0, 0, 0, // solid
    // slope 45
    0, 16, 16, 16, 16, 0, // 2
    0, 0, 16, 0, 16, 16,  // 3
    0, 0, 0, 16, 16, 16,  // 4
    0, 0, 0, 16, 16, 0,   // 5
    // slope lo
    0, 16, 16, 16, 16, 8, // 6
    0, 0, 16, 0, 16, 8,   // 7
    0, 8, 0, 16, 16, 16,  // 8
    0, 0, 16, 0, 0, 8,    // 9
    16, 0, 16, 16, 8, 16, // 10
    8, 0, 16, 0, 16, 16,  // 11
    0, 0, 0, 16, 8, 16,   // 12
    0, 0, 8, 0, 0, 16,    // 13
    // slopes hi
    0, 16, 16, 16, 16, 8, // 14
    0, 16, 16, 16, 16, 8,
    0, 0, 16, 0, 16, 8, // 15
    0, 0, 16, 0, 16, 8,
    0, 8, 0, 16, 16, 16, // 16
    0, 8, 0, 16, 16, 16,
    0, 0, 16, 0, 0, 8, // 17
    0, 0, 16, 0, 0, 8,
    16, 0, 16, 16, 8, 16, // 18
    16, 0, 16, 16, 8, 16,
    8, 0, 16, 0, 16, 16, // 19
    8, 0, 16, 0, 16, 16,
    0, 0, 0, 16, 8, 16, // 20
    0, 0, 0, 16, 8, 16,
    0, 0, 8, 0, 0, 16, // 21
    0, 0, 8, 0, 0, 16,
    //
};

// operate on 16x16 tiles
// these are the collision masks per pixel row of a tile
static const int g_pxmask_tab[][16] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    //
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    // SLOPES 45
    0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
    //
    0xFFFF, 0x7FFF, 0x3FFF, 0x1FFF, 0x0FFF, 0x07FF, 0x03FF, 0x01FF, 0x00FF, 0x007F, 0x003F, 0x001F, 0x000F, 0x0007, 0x0003, 0x0001,
    //
    0x8000, 0xC000, 0xE000, 0xF000, 0xF800, 0xFC00, 0xFE00, 0xFF00, 0xFF80, 0xFFC0, 0xFFE0, 0xFFF0, 0xFFF8, 0xFFFC, 0xFFFE, 0xFFFF,
    //
    0xFFFF, 0xFFFE, 0xFFFC, 0xFFF8, 0xFFF0, 0xFFE0, 0xFFC0, 0xFF80, 0xFF00, 0xFE00, 0xFC00, 0xF800, 0xF000, 0xE000, 0xC000, 0x8000,
    // SLOPES LO
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0003, 0x000F, 0x003F, 0x00FF, 0x03FF, 0x0FFF, 0x3FFF, 0xFFFF,
    //
    0xFFFF, 0x3FFF, 0x0FFF, 0x03FF, 0x00FF, 0x003F, 0x000F, 0x0003, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    //
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xC000, 0xF000, 0xFC00, 0xFF00, 0xFFC0, 0xFFF0, 0xFFFC, 0xFFFF,
    //
    0xFFFF, 0xFFFC, 0xFFF0, 0xFFC0, 0xFF00, 0xFC00, 0xF000, 0xC000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    // rotated
    0x0001, 0x0001, 0x0003, 0x0003, 0x0007, 0x0007, 0x000F, 0x000F, 0x001F, 0x001F, 0x003F, 0x003F, 0x007F, 0x007F, 0x00FF, 0x00FF,
    //
    0x00FF, 0x00FF, 0x007F, 0x007F, 0x003F, 0x003F, 0x001F, 0x001F, 0x000F, 0x000F, 0x0007, 0x0007, 0x0003, 0x0003, 0x0001, 0x0001,
    //
    0x8000, 0x8000, 0xC000, 0xC000, 0xE000, 0xE000, 0xF000, 0xF000, 0xF800, 0xF800, 0xFC00, 0xFC00, 0xFE00, 0xFE00, 0xFF00, 0xFF00,
    //
    0xFF00, 0xFF00, 0xFE00, 0xFE00, 0xFC00, 0xFC00, 0xF800, 0xF800, 0xF000, 0xF000, 0xE000, 0xE000, 0xC000, 0xC000, 0x8000, 0x8000,
    // SLOPES HI
    0x0003, 0x000F, 0x003F, 0x00FF, 0x03FF, 0x0FFF, 0x3FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    //
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x3FFF, 0x0FFF, 0x03FF, 0x00FF, 0x003F, 0x000F, 0x0003,
    //
    0xC000, 0xF000, 0xFC00, 0xFF00, 0xFFC0, 0xFFF0, 0xFFFC, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
    //
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFC, 0xFFF0, 0xFFC0, 0xFF00, 0xFC00, 0xF000, 0xC000,
    // rotated
    0x01FF, 0x01FF, 0x03FF, 0x03FF, 0x07FF, 0x07FF, 0x0FFF, 0x0FFF, 0x1FFF, 0x1FFF, 0x3FFF, 0x3FFF, 0x7FFF, 0x7FFF, 0xFFFF, 0xFFFF,
    //
    0xFFFF, 0xFFFF, 0x7FFF, 0x7FFF, 0x3FFF, 0x3FFF, 0x1FFF, 0x1FFF, 0x0FFF, 0x0FFF, 0x07FF, 0x07FF, 0x03FF, 0x03FF, 0x01FF, 0x01FF,
    //
    0xFF80, 0xFF80, 0xFFC0, 0xFFC0, 0xFFE0, 0xFFE0, 0xFFF0, 0xFFF0, 0xFFF8, 0xFFF8, 0xFFFC, 0xFFFC, 0xFFFE, 0xFFFE, 0xFFFF, 0xFFFF,
    //
    0xFFFF, 0xFFFF, 0xFFFE, 0xFFFE, 0xFFFC, 0xFFFC, 0xFFF8, 0xFFF8, 0xFFF0, 0xFFF0, 0xFFE0, 0xFFE0, 0xFFC0, 0xFFC0, 0xFF80, 0xFF80,
    //
};

bool32 tiles_area(tilegrid_s tg, rec_i32 r)
{
        rec_i32 rgrid = {0, 0, tg.pixel_x, tg.pixel_y};
        rec_i32 riarea;
        if (!intersect_rec(r, rgrid, &riarea)) return 0;

        int px0 = riarea.x;
        int py0 = riarea.y;
        int px1 = riarea.x + riarea.w - 1;
        int py1 = riarea.y + riarea.h - 1;
        int tx0 = px0 >> 4; // divide by 16 (tile size)
        int ty0 = py0 >> 4;
        int tx1 = px1 >> 4;
        int ty1 = py1 >> 4;

        for (int ty = ty0; ty <= ty1; ty++) {
                int y0 = (ty == ty0 ? py0 & 15 : 0); // px in tile (local)
                int y1 = (ty == ty1 ? py1 & 15 : 15);
                for (int tx = tx0; tx <= tx1; tx++) {
                        int tl = tg.tiles[tx + ty * tg.tiles_x];
                        if (tl == 0) continue;
                        if (tl == 1) return 1;
                        int x0 = (tx == tx0 ? px0 & 15 : 0);
                        int x1 = (tx == tx1 ? px1 & 15 : 15);
                        int mk = (0xFFFF >> x0) & ~(0x7FFF >> x1);
                        // mk masks the collision data so we only see
                        // the relevant part    1---5
                        // px0 = 1, px1 = 5 -> 01111100

                        for (int py = y0; py <= y1; py++)
                                if (g_pxmask_tab[tl][py] & mk) return 1;
                }
        }
        return 0;
}

bool32 tiles_area_row(tilegrid_s tg, rec_i32 r)
{
        ASSERT(r.h == 1);
        rec_i32 rgrid = {0, 0, tg.pixel_x, tg.pixel_y};
        rec_i32 riarea;
        if (!intersect_rec(r, rgrid, &riarea)) return 0;

        int px0 = riarea.x;
        int py0 = riarea.y;
        int px1 = riarea.x + riarea.w - 1;
        int tx0 = px0 >> 4; // divide by 16 (tile size)
        int tx1 = px1 >> 4;
        int ty  = py0 >> 4;
        int py  = (py0 & 15); // px in tile (local)
        for (int tx = tx0; tx <= tx1; tx++) {
                int tl = tg.tiles[tx + ty * tg.tiles_x];
                if (tl == 0) continue;
                if (tl == 1) return 1;
                int x0 = (tx == tx0 ? px0 & 15 : 0);
                int x1 = (tx == tx1 ? px1 & 15 : 15);
                int mk = (0xFFFF >> x0) & ~(0x7FFF >> x1);
                // mk masks the collision data so we only see
                // the relevant part    1---5
                // px0 = 1, px1 = 5 -> 01111100

                if (g_pxmask_tab[tl][py] & mk) return 1;
        }
        return 0;
}

bool32 tiles_at(tilegrid_s tg, i32 x, i32 y)
{
        if (!(0 <= x && x < tg.pixel_x && 0 <= y && y < tg.pixel_y)) return 0;
        int tilex = x >> 4; /* "/ 16" */
        int tiley = y >> 4;
        int t     = tg.tiles[tilex + tiley * tg.tiles_x];
        if (t <= 1) return t; // ID = 0 all 0, ID = 1 all 1
        return (g_pxmask_tab[t][y & 15] & (0x8000 >> (x & 15)));
}