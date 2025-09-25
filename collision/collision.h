#ifndef CC_COLLISION_H
#define CC_COLLISION_H

#include <core/macros/macros.h>

static inline b8 cc_collidePointRect(f32 x, f32 y, f32 rx, f32 ry, f32 rw, f32 rh) {
    return x >= rx && y >= ry && x <= rx+rw && y <= ry+rh;
}

#endif
