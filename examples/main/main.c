#include <core/platform/platform.h>

int main(void) {
    platformInit("wayland client", 800,600);
    platformDeinit();
    return 0;
}
