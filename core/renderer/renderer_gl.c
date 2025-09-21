#include "renderer.h"

#include <GLES3/gl3.h>

struct {
    // naming convention:
    //  its basically just whats easy for me to type
    //  1 - a
    //  2 - s
    //  3 - d
    //  4 - f
    //  etc.
    //  2.5 - sg

    struct {
        struct {
            u32 vao;
            s32 loc_pos;
            s32 loc_size;
            s32 loc_col;
            s32 loc_proj;
        } rect;
    } s;
} bufsgl;

f32 proj2d[16];

f32 r,g,b,a;

void* cc_gl_rendererInit(void) {
    
}

void cc_gl_rendererDeinit(void* state) {
    
}

