#include <stdio.h>
#include <core/platform/platform.h>

int main(void) {
    wlPlatformInit("wayland client", 800,600);
    wlPlatformDeinit();
    return 0;
}
