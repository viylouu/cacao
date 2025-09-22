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
    switch (renderer_api) {
        case CC_API_VULKAN: return;
        case CC_API_OPENGL: return cc_gl_rendererFlush();
    }
}
