#ifndef C_PLATFORM_H
#define C_PLATFORM_H

#include <stdint.h>
#include <stdlib.h>

int8_t wlPlatformInit(const char* title, int32_t targwidth, int32_t targheight);
int8_t wlPlatformDeinit();

#ifdef _WIN32
    fuck
#else
#define platformInit(title, targwidth, targheight) do { \
            if (getenv("WAYLAND_DISPLAY")) \
                wlPlatformInit(title, targwidth, targheight); \
            else {} \
                  \
        } while(0)

#define platformDeinit() do { \
            if (getenv("WAYLAND_DISPLAY")) \
                wlPlatformDeinit(); \
            else {} \
                  \
        } while(0)

#endif

#endif
