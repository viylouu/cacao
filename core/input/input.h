#ifndef CC_INPUT_H
#define CC_INPUT_H

#include <core/macros/macros.h>
#include <xkbcommon/xkbcommon.h>

typedef enum {
    CC_STATE_SPECIAL_RELEASED,
    CC_STATE_SPECIAL_PRESSED,

    CC_STATE_RELEASED,
    CC_STATE_PRESSED,
    CC_STATE_HELD,
    CC_STATE_INACTIVE,
} CCkeyState;

extern f64 cc_mouse_x;
extern f64 cc_mouse_y;

extern CCkeyState cc_mouse_buttons[5];

#define CC_MOUSE_LEFT 272
#define CC_MOUSE_RIGHT 273
#define CC_MOUSE_MIDDLE 274
#define CC_MOUSE_BACK 275
#define CC_MOUSE_FORTH 276

// hacky
#define CC_KEY_ XKB_KEY_

void cc_addDirtyKey(u32 key);
void cc_inputPoll(void);
void cc_inputDeinit(void);

#endif
