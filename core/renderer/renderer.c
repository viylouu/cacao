#include "renderer.h"

#include <core/platform/platform.h>

CCrendererApi renderer_api;

void* cc_rendererInit(CCrendererApi api, const char* title) {
    renderer_api = api;

    switch (renderer_api) {
        case CC_API_VULKAN: return cc_vk_rendererInit(title);
        case CC_API_OPENGL: cc_gl_rendererInit(); return 0;
    }
}

void cc_rendererUpdate(void* state, CCclientState* pstate) {
    (void)state;

    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererUpdate(pstate->width, pstate->height);
    }
}

void cc_rendererDeinit(void* state) {
    switch (renderer_api) {
        case CC_API_VULKAN: return cc_vk_rendererDeinit(state);
        case CC_API_OPENGL: return cc_gl_rendererDeinit();
    }
}

void cc_rendererFlush(void* state) {
    (void)state;

    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererFlush();
    }
}

void cc_unloadTexture(CCtexture* tex) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_unloadTexture(tex);
    }
}

CCtexture* cc_loadTexture(const char* path) {
    switch (renderer_api) {
        case CC_API_VULKAN: return NULL;
        case CC_API_OPENGL: return (CCtexture*)cc_gl_loadTexture(path);
    }
}

// FUNCS
void cc_rendererSetTint(f32 r, f32 g, f32 b, f32 a) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererSetTint(r,g,b,a);
    }
}

void cc_rendererClear(f32 r, f32 g, f32 b, f32 a) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererClear(r,g,b,a);
    }
}

void cc_rendererResetTransform(void) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererResetTransform();
    }
}

void cc_rendererGetTransform(mat4* out) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererGetTransform(out);
    }
}

void cc_rendererSetTransform(mat4* matrix) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererSetTransform(matrix);
    }
}

void cc_rendererTranslate(float x, float y, float z) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererTranslate(x,y,z);
    }
}

void cc_rendererScale(float x, float y, float z) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererScale(x,y,z);
    }
}

void cc_rendererRotate(float x, float y, float z) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererRotate(x,y,z);
    }
}

//     2D
void cc_rendererDrawRect(f32 x, f32 y, f32 w, f32 h) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererDrawRect(x,y,w,h);
    }
}

void cc_rendererDrawTexture(CCtexture* tex, f32 x, f32 y, f32 w, f32 h, f32 sx, f32 sy, f32 sw, f32 sh) {
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererDrawTexture(tex, x,y,w,h,sx,sy,sw,sh);
    }
}
