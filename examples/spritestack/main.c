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

    char buf[1024];
    sprintf(buf, "almighty minecraft car cube item\n%4.2f FPS", 1.f/cc_delta);

    cc_rendererRotateSpriteStackCamera(-cc_time);
    cc_rendererTranslateSpriteStackCamera(cc_width/2, cc_height/2, sin(cc_time*2)*128);

    cc_rendererSetTint(1,1,1,1);

    s32 size = 6;

    for (s32 x = -size; x < size; ++x)
        for (s32 y = -size; y < size; ++y)
            for (s32 z = -size; z < size; ++z)
                cc_rendererDrawSpriteStack(stack, x*16, y*16, z*16, 2, 0);

    cc_textDrawText(font, buf, 3, 8,8);
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

