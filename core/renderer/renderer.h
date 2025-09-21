#ifndef CC_RENDERER_H
#define CC_RENDERER_H

// table of contents:
//  VULKAN
//  GL
//      SHADERS
//      MAIN
//      FUNCS
//  GENERAL

#include <core/macros/macros.h>
#include <core/platform/platform.h>

extern b8 cc_renderer_use_wayland;

//
// VULKAN
//

void* cc_vk_rendererInit(const char* title);
void cc_vk_rendererDeinit(void* state);

//
// GL
//

void cc_gl_load(void);

// SHADERS
u32 cc_gl_compileProgram(u32* shaders, u32 amount);
u32 cc_gl_loadShaderFromSource(u32 type, const char** source);
u32 cc_gl_loadProgramFromSource(const char** vert, const char** frag);
char* loadShaderSrc(const char* path);
u32 cc_gl_loadShader(u32 type, const char* path);
u32 cc_gl_loadProgram(const char* vert, const char* frag);

// MAIN
void cc_gl_rendererInit(void);
void cc_gl_rendererUpdate(s32 width, s32 height);
void cc_gl_rendererDeinit(void);

// FUNCS
void cc_gl_rendererSetTint(f32 red, f32 green, f32 blue, f32 alpha);
void cc_gl_rendererDrawRect(f32 x, f32 y, f32 w, f32 h);

//
// GENERAL
//

void* cc_rendererInit(CCrendererApi api, const char* title);
void cc_rendererUpdate(void* state, CCclientState* pstate);
void cc_rendererDeinit(void* state);

#endif
