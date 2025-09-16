#ifndef CC_PLATFORM_H
#define CC_PLATFORM_H

#include <core/macros/macros.h>

extern void (*cc_wl_drawFunction)(void);
extern u16 cc_wl_width;
extern u16 cc_wl_height;
extern u8 cc_wl_tick;
extern u8 cc_wl_shouldWindowClose;

s8 cc_wl_platformInit(const char* title, s32 targwidth, s32 targheight);
s8 cc_wl_platformGetShouldWindowClose(void);
s8 cc_wl_platformDeinit(void);

extern u8 cc_x_shouldWindowClose;

s8 cc_x_platformInit(const char* title, s32 targwidth, s32 targheight);
s8 cc_x_platformGetShouldWindowClose(void);
s8 cc_x_platformDeinit(void);

void cc_platformInit(const char* title, s32 targwidth, s32 targheight);
void cc_platformSetDrawFunc(void (*func)(void));
s8 cc_platformShouldWindowClose(void);
void cc_platformCloseWindow(void);
void cc_platformDeinit(void);

#endif
