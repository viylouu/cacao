#include <core/engine.h>
#include <core/renderer/renderer.h>

typedef struct {
    CCtexture* texture;
    u32 layers;
    u32 layerHeight;
} CCspriteStack;

CCspriteStack* stack;

void init(void) {
    stack = malloc(sizeof(CCspriteStack));
    stack->texture = cc_loadTexture("examples/spritestack/car.png");
    stack->layerHeight = 13;
    stack->layers = stack->texture->height / stack->layerHeight;
}

void update(void) {}

void render(void) {
    cc_rendererClear(.2,.3,.4,1);

    cc_rendererSetTint(1,1,1,1);

    f32 x, y, scale;
    x = cc_width / 2;
    y = cc_height / 2;
    scale = 8;

    cc_rendererScale(scale,scale,1);

    mat4 prev;
    cc_rendererGetTransform(&prev);

    for (f32 i = stack->layers-1; i >= 0; i -= 1.f/scale) {
        cc_rendererRotate(0,0,45*(3.14159256f/180.f));

        cc_rendererTranslate(x, y, 0);
        cc_rendererTranslate(0,i*scale,0);

        cc_rendererDrawTexture(stack->texture, 0,0,stack->texture->width,stack->layerHeight, 0,(s32)i*stack->layerHeight,stack->texture->width,stack->layerHeight);
        cc_rendererSetTransform(&prev);
    }
}

void clean(void) {
    cc_unloadTexture(stack->texture);
    free(stack);
}

int main(void) {
    return cc_engineMain(
        "MAIN",
        800, 600,
        init, update,
        render, clean
        );
}

