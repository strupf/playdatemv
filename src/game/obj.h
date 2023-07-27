#ifndef OBJ_H
#define OBJ_H

#include "gamedef.h"

struct objhandle_s {
        int    gen;
        obj_s *o;
};

struct obj_s {
        int gen;
        int index;

        obj_s *parent;
        obj_s *next;
        obj_s *prev;
        obj_s *fchild;
        obj_s *lchild;

        i32    w;
        i32    h;
        v2_i32 pos;
        v2_i32 subpos_q8;
        v2_i32 vel_q8;
        v2_i32 gravity_q8;
        v2_i32 drag_q8;
};

bool32      objhandle_is_valid(objhandle_s h);
bool32      objhandle_is_null(objhandle_s h);
obj_s      *obj_from_objhandle(objhandle_s h);
objhandle_s objhandle_from_obj(obj_s *o);
//
obj_s      *obj_create(game_s *g);
void        obj_delete(game_s *g, obj_s *o);
rec_i32     obj_aabb(obj_s *o);

#endif