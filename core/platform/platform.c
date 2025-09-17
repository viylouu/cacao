#include "platform.h"

#include <core/macros/macros.h>
#include <stdlib.h>

s8 g_is_wayland;

void* cc_platformInit(const char* title, s32 targwidth, s32 targheight) {
#ifdef _WIN32
#else
    g_is_wayland = getenv("WAYLAND_DISPLAY") != 0;
    if (g_is_wayland)
        return cc_wl_platformInit(title, targwidth, targheight);
    else {} 
       // cc_x_platformInit(title, targwidth, targheight);
    
#endif

    return 0;
}

void cc_platformDeinit(void* data) {
#ifdef _WIN32
#else
    if (g_is_wayland)
        cc_wl_platformDeinit(data);
    else {}
        //cc_x_platformDeinit();
#endif
}

s8 cc_platformShouldClose(void* data) {
#ifdef _WIN32
#else
    if (g_is_wayland)
        return !cc_wl_platformIsRunning(data);
    else {}
        //return cc_x_platformGetShouldWindowClose();
#endif
    return 1;
}

void cc_platformCloseWindow(void* data) {
#ifdef _WIN32
#else
    if (g_is_wayland)
        ((CCclientState*)data)->running = 0;
    else {}
        //cc_x_shouldWindowClose = 1;
#endif
}

void cc_platformSwapBuffer(void* data){
#ifdef _WIN32
#else
    if (g_is_wayland) {

    }
#endif
}
