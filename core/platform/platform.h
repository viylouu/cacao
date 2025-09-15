#ifndef CC_PLATFORM_H
#define CC_PLATFORM_H

#include <stdint.h>
#include <stdlib.h>

extern void (*cc_wl_drawFunction)(void);
extern uint16_t cc_wl_width;
extern uint16_t cc_wl_height;
extern uint8_t cc_wl_tick;
extern uint8_t cc_wl_shouldWindowClose;

int8_t cc_wl_platformInit(const char* title, int32_t targwidth, int32_t targheight);
int8_t cc_wl_getShouldWindowClose(void);
int8_t cc_wl_platformDeinit();

void cc_platformInit(const char* title, int32_t targwidth, int32_t targheight);
void cc_platformSetDrawFunc(void (*func)(void));
int8_t cc_platformShouldWindowClose(void);
void cc_platformCloseWindow(void);
void cc_platformDeinit(void);

#endif
