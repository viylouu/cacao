#include <core/engine.h>
#include <core/renderer/renderer.h>
#include <core/types/text.h>
#include <stdio.h>
#include <math.h>

CCspriteStack* stack;
CCfont* font;

void init(void) {
    stack = cc_loadSpriteStack("examples/spritestack/car.png", 13);
    font = cc_textLoadFont("data/eng/font.png");
}

void update(void) {}

void render(void) {
    cc_rendererClear(.2,.3,.4,1);

    char buf[32];
    sprintf(buf, "%4.2f FPS", 1.f/cc_delta);

    cc_rendererSetTint(1,1,1,1);
    cc_textDrawText(font, buf, 3, 0,0);

    for (s32 x = -8; x < 8; ++x)
        for (s32 y = -8; y < 8; ++y)
            for (s32 z = -8; z < 8; ++z)
                cc_rendererDrawSpriteStack(stack, cc_width/2/2 + x*16 + z*8, cc_height/2/2 + y*16, z*16, 2, cc_time);
}

void clean(void) {
    cc_unloadSpriteStack(stack);
    cc_textUnloadFont(font);
}

int main(void) {
    return cc_engineMain(
        "MAIN",
        800, 600,
        init, update,
        render, clean
        );
}

