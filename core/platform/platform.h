#ifndef C_PLATFORM_H
#define C_PLATFORM_H

#include <stdint.h>

int8_t wlPlatformInit(const char* title, int32_t targwidth, int32_t targheight);
int8_t wlPlatformDeinit();

#endif
