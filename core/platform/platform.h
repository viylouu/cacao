#ifndef CC_PLATFORM_H
#define CC_PLATFORM_H

#include <core/macros/macros.h>

typedef struct {
    s32 width;
    s32 height;
    s32 stride;
    s32 pool_size;
    s32 size;
    s8 running;
} CCclientState;

void* cc_wl_platformInit(const char* title, s32 targwidth, s32 targheight);
s8 cc_wl_platformIsRunning(void* state);
s8 cc_wl_platformDeinit(void* state);

void* cc_platformInit(const char* title, s32 targwidth, s32 targheight);
void cc_platformDeinit(void* data);
s8 cc_platformShouldClose(void* data);
void cc_platformCloseWindow(void* data);
void cc_platformSwapBuffer(void* data);

#endif
