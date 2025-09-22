#ifndef CC_TEXT_H
#define CC_TEXT_H

#include <core/renderer/renderer.h>

typedef struct {
    int charW, charH;
    CCtexture* atlas;
} CCfont;

CCfont* cc_textLoadFont(const char* file);
void cc_textUnloadFont(CCfont* font);
void cc_textDrawText(CCfont* font, const char* text, float size, float x, float y);

#endif
