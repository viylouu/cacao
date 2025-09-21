#ifndef CC_PLATFORM_H
#define CC_PLATFORM_H

#include <core/macros/macros.h>

typedef struct {
    s32 width;
    s32 height;
    s32 stride;
    s32 pool_size;
    s32 size;
    b8 running;
    b8 fullscreen;
} CCclientState;

typedef enum {
    CC_API_VULKAN,
    CC_API_OPENGL
} CCrendererApi;

void* cc_wl_platformInit(CCrendererApi api, const char* title, s32 targwidth, s32 targheight);
s8 cc_wl_platformIsRunning(void* state);
void cc_wl_platformDeinit(void* state);
void cc_wl_platformSwapBuffers(void* client);

void* cc_platformInit(CCrendererApi api, const char* title, s32 targwidth, s32 targheight);
void cc_platformDeinit(void* data);
s8 cc_platformShouldClose(void* data);
void cc_platformCloseWindow(void* data);
void cc_platformSwapBuffer(void* data);

#endif
