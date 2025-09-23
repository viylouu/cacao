#include "text.h"

#include <core/renderer/renderer.h>

CCfont* cc_textLoadFont(const char* file) {
    CCfont* font = malloc(sizeof(CCfont));
    font->atlas = cc_loadTexture(file);
    font->charW = font->atlas->width/16;
    font->charH = font->atlas->height/16;
    return font;
}

void cc_textUnloadFont(CCfont* font) {
    cc_unloadTexture(font->atlas);
    free(font);
}

void cc_textDrawText(CCfont* font, const char* text, f32 size, f32 x, f32 y) {
    s32 charX = 0, charY = 0;
    for (s32 i = 0; text[i] != '\0'; ++i) {
        char cur = text[i];
        s32 sx = cur >> 4;
        s32 sy = cur & 0xF;

        cc_gl_rendererDrawTexture(
            font->atlas,
            x + charX * font->charW * size,
            y + charY * font->charH * size,
            font->charW * size,
            font->charH * size,
            sx * font->charW,
            sy * font->charH,
            font->charW,
            font->charH
            );

        ++charX;
        if (text[i] == '\n') {
            ++charY;
            charX = 0;
        }
    }
}
