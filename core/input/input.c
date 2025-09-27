#include "input.h"

#include <stdlib.h>
#include <xkbcommon/xkbcommon.h>
#include <stdio.h>

f64 cc_mouse_x;
f64 cc_mouse_y;

CCkeyState cc_keyboard_keys[0x100000fd]; 

u32* dirty_keys = NULL;
u64 dirty_key_amt = 0;

void cc_addDirtyKey(u32 key) {
    dirty_keys = realloc(dirty_keys, (dirty_key_amt + 1) * sizeof(u32));
    dirty_keys[dirty_key_amt++] = key;
}

void cc_inputPoll(void) {
    for (u32 i = 0; i < dirty_key_amt; ++i) {
        u32 k = dirty_keys[i];

        if (cc_keyboard_keys[k] == CC_STATE_HELD || cc_keyboard_keys[k] == CC_STATE_RELEASED) {
            dirty_keys[i] = dirty_keys[dirty_key_amt - 1];
            dirty_key_amt--;
            continue;
        }

        if (cc_keyboard_keys[k] == CC_STATE_PRESSED) cc_keyboard_keys[k] = CC_STATE_HELD;
        else if (cc_keyboard_keys[k] == CC_STATE_RELEASED) cc_keyboard_keys[k] = CC_STATE_INACTIVE;
        else if (cc_keyboard_keys[k] == CC_STATE_SPECIAL_PRESSED) cc_keyboard_keys[k] = CC_STATE_PRESSED;
        else if (cc_keyboard_keys[k] == CC_STATE_SPECIAL_RELEASED) cc_keyboard_keys[k] = CC_STATE_RELEASED;
    }

    /*for (u32 i = 0; i < 5; ++i) {
        if (cc_mouse_buttons[i] == CC_STATE_PRESSED) cc_mouse_buttons[i] = CC_STATE_HELD;
        else if (cc_mouse_buttons[i] == CC_STATE_RELEASED) cc_mouse_buttons[i] = CC_STATE_INACTIVE;
        else if (cc_mouse_buttons[i] == CC_STATE_SPECIAL_PRESSED) cc_mouse_buttons[i] = CC_STATE_PRESSED;
        else if (cc_mouse_buttons[i] == CC_STATE_SPECIAL_RELEASED) cc_mouse_buttons[i] = CC_STATE_RELEASED;
    }*/
}

void cc_inputDeinit(void) {
    free(dirty_keys);
    dirty_keys = NULL;
    dirty_key_amt = 0;
}

