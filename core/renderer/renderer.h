#ifndef CC_RENDERER_H
#define CC_RENDERER_H

#include <core/macros/macros.h>


void* cc_vk_rendererInit(const char* title);
void cc_vk_rendererDeinit(void* state);


typedef enum {
    CC_API_VULKAN
} CCrendererApi;

void* cc_rendererInit(CCrendererApi api, const char* title);
void cc_rendererDeinit(void* state);

#endif
