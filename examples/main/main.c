#include <core/engine.h>
#include <core/input/input.h>
#include <xkbcommon/xkbcommon.h>
#include <core/renderer/renderer.h>

void init(void) {}

void update(void) {
    //printf("checking key %d\n", XKB_KEY_A);

    printf("%d ", (s32)(1.f/cc_delta));
    if (cc_isKeyHeld(XKB_KEY_A))
        printf("key a pressed\n");
}

#include <GLES2/gl2.h>
void render(void) {
    glClearColor(.2,.3,.4,1);
    glClear(GL_COLOR_BUFFER_BIT);

    cc_gl_rendererSetTint(1,0,0,1);
    cc_gl_rendererDrawRect(0,0,64,64);
}

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
