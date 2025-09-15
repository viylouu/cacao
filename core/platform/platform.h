#ifndef CC_PLATFORM_H
#define CC_PLATFORM_H

#include <stdint.h>
#include <stdlib.h>

int8_t wl_platformInit(const char* title, int32_t targwidth, int32_t targheight);
int8_t wl_platformDeinit();

#ifdef _WIN32
    fuck
#else
#define platformInit(title, targwidth, targheight) do { \
            if (getenv("WAYLAND_DISPLAY")) \
                wl_platformInit(title, targwidth, targheight); \
            else {} \
                  \
        } while(0)

#define platformDeinit() do { \
            if (getenv("WAYLAND_DISPLAY")) \
                wl_platformDeinit(); \
            else {} \
                  \
        } while(0)

#endif

#endif
