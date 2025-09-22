#ifndef CC_RENDERER_H
#define CC_RENDERER_H

// table of contents:
//  GENERAL
//  VULKAN
//  GL
//      TEXTURES
//      SHADERS
//      MAIN
//      FUNCS
//          2D
// 

#include <core/macros/macros.h>
#include <core/platform/platform.h>
#include <stdlib.h>

extern b8 cc_renderer_use_wayland;



//
// GENERAL
//

typedef struct {
    void* platform_specific;
    s32 width;
    s32 height;
} CCtexture;

void* cc_rendererInit(CCrendererApi api, const char* title);
void cc_rendererUpdate(void* state, CCclientState* pstate);
void cc_rendererDeinit(void* state);
void cc_rendererFlush(void* state);

void cc_unloadTexture(CCtexture* tex);
CCtexture* cc_loadTexture(const char* path);

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

// MAIN
void cc_gl_rendererInit(void);
void cc_gl_rendererUpdate(s32 width, s32 height);
void cc_gl_rendererDeinit(void);

// FUNCS
void cc_gl_rendererFlush(void);
void cc_gl_rendererSetTint(f32 red, f32 green, f32 blue, f32 alpha);

//   2D
void cc_gl_rendererDrawRect(f32 x, f32 y, f32 w, f32 h);
void cc_gl_rendererDrawTexture(CCtexture* tex, f32 x, f32 y, f32 w, f32 h, f32 sx, f32 sy, f32 sw, f32 sh);

#endif
