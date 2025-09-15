#include "platform.h"

#include <stdint.h>
#include <stdlib.h>

int8_t g_is_wayland;

void cc_platformInit(const char* title, int32_t targwidth, int32_t targheight) {
#ifdef _WIN32
#else
    g_is_wayland = getenv("WAYLAND_DISPLAY") != 0;
    if (g_is_wayland)
        cc_wl_platformInit(title, targwidth, targheight);
    else {}
#endif
}

void cc_platformSetDrawFunc(void (*func)(void)) {
#ifdef _WIN32
#else
    if (g_is_wayland)
        cc_wl_drawFunction = func;
    else {}
#endif
} 

void cc_platformDeinit(void) {
#ifdef _WIN32
#else
    if (g_is_wayland)
        cc_wl_platformDeinit();
    else {}
#endif
}

int8_t cc_platformShouldWindowClose(void) {
#ifdef _WIN32
#else
    if (g_is_wayland)
        return cc_wl_getShouldWindowClose();
    else {}
#endif
    return -1;
}

void cc_platformCloseWindow(void) {
#ifdef _WIN32
#else
    if (g_is_wayland)
        cc_wl_shouldWindowClose = 1;
    else {}
#endif
}
