#ifndef CC_ENGINE_H
#define CC_ENGINE_H

#include <core/macros/macros.h>

extern f64 cc_time;
extern f64 cc_delta;

extern f32 cc_width;
extern f32 cc_height;

s32 cc_engineMain(
    const char* title,
    s32 width, s32 height,
    void (*init)(void),
    void (*update)(void),
    void (*render)(void),
    void (*clean)(void)
);

#endif
