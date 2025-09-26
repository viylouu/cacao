#include <core/engine.h>
#include <core/renderer/renderer.h>

CCspriteStack* stack;

void init(void) {
    stack = cc_loadSpriteStack("examples/spritestack/car.png", 13);
}

void update(void) {}

void render(void) {
    cc_rendererClear(.2,.3,.4,1);

    cc_rendererSetTint(1,1,1,1);

    cc_rendererDrawSpriteStack(stack, cc_width/2, cc_height/2, 0, 8, 45*(3.14159265f/180.f));
}

void clean(void) {
    cc_unloadSpriteStack(stack);
}

int main(void) {
    return cc_engineMain(
        "MAIN",
        800, 600,
        init, update,
        render, clean
        );
}

