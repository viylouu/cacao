#include "text.h"

CCfont* cc_textLoadFont(const char* file) {
    CCfont* font = malloc(sizeof(CCfont));
    font->atlas = cc_loadTexture(file);
    font->charW = font->atlas->width/16;
    font->charH = font->atlas->height/16;
}

void cc_textUnloadFont(CCfont* font) {
    
}

void cc_textDrawText(CCfont* font, const char* text, float size, float x, float y) {

}
