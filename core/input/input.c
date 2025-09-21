#include "input.h"

#include <stdlib.h>
#include <xkbcommon/xkbcommon.h>

f64 cc_mouse_x;
f64 cc_mouse_y;

CCkeyState keys[0x100000fd]; // do not fucking iterate over this!!!!!
CCkeyState cc_mouse_buttons[5];

u32* dirty_keys = NULL;
u64 dirty_key_amt = 0;

void cc_addDirtyKey(u32 key) {
    dirty_keys = realloc(dirty_keys, (dirty_key_amt + 1) * sizeof(u32));
    dirty_keys[dirty_key_amt++] = key;
}

void cc_inputPoll(void) {
    for (u32 i = 0; i < dirty_key_amt; ++i) {
        u32 k = dirty_keys[i];

        if (keys[k] == CC_STATE_HELD || keys[k] == CC_STATE_RELEASED) {
            dirty_keys[i] = dirty_keys[dirty_key_amt - 1];
            dirty_key_amt--;
        }

        if (keys[k] == CC_STATE_PRESSED) keys[k] = CC_STATE_HELD;
        else if (keys[k] == CC_STATE_RELEASED) keys[k] = CC_STATE_INACTIVE;
        else if (keys[k] == CC_STATE_SPECIAL_PRESSED) keys[k] = CC_STATE_PRESSED;
        else if (keys[k] == CC_STATE_SPECIAL_RELEASED) keys[k] = CC_STATE_RELEASED;
    }

    for (u32 i = 0; i < 5; ++i) {
        if (cc_mouse_buttons[i] == CC_STATE_PRESSED) cc_mouse_buttons[i] = CC_STATE_HELD;
        else if (cc_mouse_buttons[i] == CC_STATE_RELEASED) cc_mouse_buttons[i] = CC_STATE_INACTIVE;
        else if (cc_mouse_buttons[i] == CC_STATE_SPECIAL_PRESSED) cc_mouse_buttons[i] = CC_STATE_PRESSED;
        else if (cc_mouse_buttons[i] == CC_STATE_SPECIAL_RELEASED) cc_mouse_buttons[i] = CC_STATE_RELEASED;
    }
}

void cc_inputDeinit(void) {
    free(dirty_keys);
    dirty_keys = NULL;
    dirty_key_amt = 0;
}
