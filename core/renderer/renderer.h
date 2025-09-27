#ifndef CC_RENDERER_H
#define CC_RENDERER_H

// table of contents:
//  GENERAL
//      FUNCS
//          2D
//  VULKAN
//  GL
//      TEXTURES
//      SHADERS
//      SPRITESTACKS
//      MAIN
//      FUNCS
//          2D
//          SPRITESTACK
// 

#include <core/macros/macros.h>
#include <core/platform/platform.h>
#include <stdlib.h>
#include <core/types/mat4.h>

extern b8 cc_renderer_use_wayland;



//
// GENERAL
//

extern u32 cc_renderer_draw_calls;
extern u32 cc_gl_renderer_draw_calls;
extern u32 cc_vk_renderer_draw_calls;

typedef struct {
    void* platform_specific;
    s32 width;
    s32 height;
} CCtexture;

typedef struct {
    void* platform_specific;
    CCtexture* texture;
    u32 layers;
    s32 layer_height;
} CCspriteStack;

void* cc_rendererInit(CCrendererApi api, const char* title);
void cc_rendererUpdate(void* state, f32 width, f32 height);
void cc_rendererDeinit(void* state);
void cc_rendererFlush(void* state);

void cc_unloadTexture(CCtexture* tex);
CCtexture* cc_loadTexture(const char* path);

void cc_unloadSpriteStack(CCspriteStack* stack);
CCspriteStack* cc_loadSpriteStack(const char* path, u32 layerheight);

// FUNCS
void cc_rendererSetTint(f32 r, f32 g, f32 b, f32 a);
void cc_rendererClear(f32 r, f32 g, f32 b, f32 a);

void cc_rendererResetTransform(void);
void cc_rendererGetTransform(mat4* out);
void cc_rendererSetTransform(mat4* matrix);
void cc_rendererTranslate(float x, float y, float z);
void cc_rendererScale(float x, float y, float z);
void cc_rendererRotate(float x, float y, float z);

void cc_rendererResetSpriteStackCamera(void);
void cc_rendererTranslateSpriteStackCamera(float x, float y, float z);
void cc_rendererRotateSpriteStackCamera(float ang);
void cc_rendererTiltSpriteStackCamera(float tilt);

//     2D
void cc_rendererDrawRect(f32 x, f32 y, f32 w, f32 h);
void cc_rendererDrawTexture(CCtexture* tex, f32 x, f32 y, f32 w, f32 h, f32 sx, f32 sy, f32 sw, f32 sh);

//     SPRITESTACK
void cc_rendererDrawSpriteStack(CCspriteStack* stack, f32 x, f32 y, f32 z, f32 scale, f32 rotation);

//
// VULKAN
//

void* cc_vk_rendererInit(const char* title);
void cc_vk_rendererDeinit(void* state);

//
// GL
//

void cc_gl_load(void);

// TEXTURES
typedef struct {
    u32 id;
} GLtexture;

void cc_gl_unloadTexture(CCtexture* tex);
CCtexture* cc_gl_loadTextureFromData(u8* data, size_t size);
char* cc_gl_loadTextureData(const char* path, size_t* out_size);
CCtexture* cc_gl_loadTexture(const char* path);

// SHADERS
u32 cc_gl_compileProgram(u32* shaders, u32 amount);
u32 cc_gl_loadShaderFromSource(u32 type, const char** source);
u32 cc_gl_loadProgramFromSource(const char** vert, const char** frag);
char* cc_gl_loadShaderSource(const char* path);
u32 cc_gl_loadShader(u32 type, const char* path);
u32 cc_gl_loadProgram(const char* vert, const char* frag);

// SPRITESTACKS
void cc_gl_unloadSpriteStack(CCspriteStack* stack);
CCspriteStack* cc_gl_loadSpriteStack(const char* path, f32 layerheight);

// MAIN
void cc_gl_rendererInit(void);
void cc_gl_rendererUpdate(s32 width, s32 height);
void cc_gl_rendererDeinit(void);

// FUNCS
void cc_gl_rendererFlush(void);
void cc_gl_rendererSetTint(f32 red, f32 green, f32 blue, f32 alpha);
void cc_gl_rendererClear(f32 r, f32 g, f32 b, f32 a);

void cc_gl_rendererResetTransform(void);
void cc_gl_rendererGetTransform(mat4* out);
void cc_gl_rendererSetTransform(mat4* matrix);
void cc_gl_rendererTranslate(float x, float y, float z);
void cc_gl_rendererScale(float x, float y, float z);
void cc_gl_rendererRotate(float x, float y, float z);

void cc_gl_rendererResetSpriteStackCamera(void);
void cc_gl_rendererTranslateSpriteStackCamera(float x, float y, float z);
void cc_gl_rendererRotateSpriteStackCamera(float ang);
void cc_gl_rendererTiltSpriteStackCamera(float tilt);

//   2D
void cc_gl_rendererDrawRect(f32 x, f32 y, f32 w, f32 h);
void cc_gl_rendererDrawTexture(CCtexture* tex, f32 x, f32 y, f32 w, f32 h, f32 sx, f32 sy, f32 sw, f32 sh);

//   SPRITESTACK
void cc_gl_rendererDrawSpriteStack(CCspriteStack* stack, f32 x, f32 y, f32 z, f32 scale, f32 rotation);

#endif
