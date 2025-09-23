#include <core/engine.h>
#include <core/renderer/renderer.h>

CCtexture* tex;

void init(void) {
    tex = cc_loadTexture("examples/texture/sprite.png");
}

void update(void) {}

void render(void) {
    cc_rendererClear(.2,.3,.4,1);

    cc_rendererSetTint(1,1,1,1);
    cc_rendererDrawTexture(tex, 0,0,64,64, 0,0,tex->width, tex->height);
}

void clean(void) {
    cc_unloadTexture(tex);
}

int main(void) {
    return cc_engineMain(
        "MAIN",
        800, 600,
        init, update,
        render, clean
        );
}

