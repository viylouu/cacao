#include <core/engine.h>
#include <stdio.h>

void init(void) {}

void update(void) {
    //printf("%d ", (s32)(1.f/cc_delta));
}

void render(void) {}
void clean(void) {}

int main(void) {
    /*
    cc_platformInit("wayland client", 800,600);
    cc_platformSetDrawFunc(rend);
    while(!cc_platformShouldWindowClose()) {}
    cc_platformDeinit();
    return 0;
    */

    return cc_engineMain(
        "MAIN",
        800, 600,
        init, update,
        render, clean
        );
}
