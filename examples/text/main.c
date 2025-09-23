#include <core/engine.h>
#include <core/input/input.h>
#include <xkbcommon/xkbcommon.h>
#include <core/renderer/renderer.h>
#include <core/types/text.h>

CCfont* font;

void init(void) {
    font = cc_textLoadFont("data/eng/font.png");
}

void update(void) {}

#include <GL/gl.h>

void render(void) {
    glClearColor(.2,.3,.4,1);
    glClear(GL_COLOR_BUFFER_BIT);

    cc_gl_rendererSetTint(1,0,0,1);
    cc_textDrawText(font, "hello world :DDD\n\nthis is a...\n...\nnewline!", 2, 6,7);
}

void clean(void) {
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

