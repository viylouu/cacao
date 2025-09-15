#include <core/platform/platform.h>

void rend(void) {}

int main(void) {
    cc_platformInit("wayland client", 800,600);
    cc_platformSetDrawFunc(rend);
    while(!cc_platformShouldWindowClose()) {}
    cc_platformDeinit();
    return 0;
}
