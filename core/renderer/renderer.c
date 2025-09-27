#include "renderer.h"

#include <core/platform/platform.h>

#define call_exp(name, ...) do { \
    switch (renderer_api) { \
        case CC_API_VULKAN: return NULL; \
        case CC_API_OPENGL: return cc_gl_##name(__VA_ARGS__); \
    } \
} while(0)

#define call_ex(name, ...) do { \
    switch (renderer_api) { \
        case CC_API_VULKAN: return; \
        case CC_API_OPENGL: return cc_gl_##name(__VA_ARGS__); \
    } \
} while(0)



// in this scenario, ts stands for "this shit" and not this, or typescript
// it will never stand for typescript
#define call_ts(name, ...) call_ex(renderer##name, __VA_ARGS__)
#define call_tsp(name, ...) call_exp(renderer##name, __VA_ARGS__)
    


CCrendererApi renderer_api;

u32 cc_renderer_draw_calls;

void* cc_rendererInit(CCrendererApi api, const char* title) {
    renderer_api = api;

    switch (renderer_api) {
        case CC_API_VULKAN: return cc_vk_rendererInit(title);
        case CC_API_OPENGL: cc_gl_rendererInit(); return 0;
    }
}

void cc_rendererUpdate(void* state, f32 width, f32 height) {
    (void)state;

    if (cc_gl_renderer_draw_calls != 0)
        cc_renderer_draw_calls = cc_gl_renderer_draw_calls;
    else
        cc_renderer_draw_calls = cc_vk_renderer_draw_calls;

    cc_gl_renderer_draw_calls = 0;
    cc_vk_renderer_draw_calls = 0;

    call_ts(Update, width, height);
}

void cc_rendererDeinit(void* state) {
    switch (renderer_api) {
        case CC_API_VULKAN: return cc_vk_rendererDeinit(state);
        case CC_API_OPENGL: return cc_gl_rendererDeinit();
    }
}

void cc_rendererFlush(void* state) { (void)state;  call_ts(Flush); }

void cc_unloadTexture(CCtexture* tex) {  call_ex(unloadTexture, tex); }
CCtexture* cc_loadTexture(const char* path) { call_exp(loadTexture, path); }

void cc_unloadSpriteStack(CCspriteStack* stack) { call_ex(unloadSpriteStack, stack); }
CCspriteStack* cc_loadSpriteStack(const char* path, u32 layerheight) { call_exp(loadSpriteStack, path, layerheight); }

// FUNCS
void cc_rendererSetTint(f32 r, f32 g, f32 b, f32 a) { call_ts(SetTint, r,g,b,a); }

void cc_rendererClear(f32 r, f32 g, f32 b, f32 a) { call_ts(Clear, r,g,b,a); }

void cc_rendererResetTransform(void) { call_ts(ResetTransform); }
void cc_rendererGetTransform(mat4* out) { call_ts(GetTransform, out); }
void cc_rendererSetTransform(mat4* matrix) { call_ts(SetTransform, matrix); }

void cc_rendererTranslate(float x, float y, float z) { call_ts(Translate, x,y,z); }
void cc_rendererScale(float x, float y, float z) { call_ts(Scale, x,y,z); }
void cc_rendererRotate(float x, float y, float z) { call_ts(Rotate, x,y,z); }

void cc_rendererResetSpriteStackCamera(void) { call_ts(ResetSpriteStackCamera); }
void cc_rendererTranslateSpriteStackCamera(float x, float y, float z) { call_ts(TranslateSpriteStackCamera, x,y,z); }
void cc_rendererScaleSpriteStackCamera(float scale) { call_ts(ScaleSpriteStackCamera, scale); }
void cc_rendererRotateSpriteStackCamera(float ang) { call_ts(RotateSpriteStackCamera, ang); }
void cc_rendererTiltSpriteStackCamera(float tilt) { call_ts(TiltSpriteStackCamera, tilt); }

//     2D
void cc_rendererDrawRect(f32 x, f32 y, f32 w, f32 h) { call_ts(DrawRect, x,y,w,h); }
void cc_rendererDrawTexture(CCtexture* tex, f32 x, f32 y, f32 w, f32 h, f32 sx, f32 sy, f32 sw, f32 sh) { call_ts(DrawTexture,tex,x,y,w,h,sx,sy,sw,sh); }

//     SPRITESTACK
void cc_rendererDrawSpriteStack(CCspriteStack* stack, f32 x, f32 y, f32 z, f32 rotation) { call_ts(DrawSpriteStack, stack,x,y,z,rotation); }
