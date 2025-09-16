#include "platform.h"

// thanks to Codotaku on his video for a basic window https://www.youtube.com/watch?v=IPGROgWnI_8
// also
// wayland better

// TODO:actually finish this

#include <xcb/xcb.h>
#include <stdio.h>

xcb_connection_t* g_x_connection;
const xcb_setup_t* g_x_setup;
xcb_screen_iterator_t g_x_screen_iterator;
xcb_screen_t* g_x_screen;
xcb_window_t g_x_window;
s32 g_x_screen_number;

u8 cc_x_shouldWindowClose;

s32 cc_x_width = 200;
s32 cc_x_height = 100;

s8 cc_x_platformInit(const char* title, s32 targwidth, s32 targheight) {
    (void)title; // for now

    cc_x_width = targwidth;
    cc_x_height = targheight;

    g_x_connection = xcb_connect(NULL, &g_x_screen_number);
    if (xcb_connection_has_error(g_x_connection)) { printf("failed to connect to display server!\n"); return 1; }

    g_x_setup = xcb_get_setup(g_x_connection);
    g_x_screen_iterator = xcb_setup_roots_iterator(g_x_setup);
    for (int i = 0; i < g_x_screen_number; ++i) xcb_screen_next(&g_x_screen_iterator);
    g_x_screen = g_x_screen_iterator.data;

    g_x_window = xcb_generate_id(g_x_connection);
    xcb_create_window(
            g_x_connection,
            g_x_screen->root_depth,
            g_x_window,
            g_x_screen->root,
            0,0,
            cc_x_width,
            cc_x_height,
            0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            g_x_screen->root_visual,
            XCB_EVENT_MASK_NO_EVENT, &(s32[]){}
            );

    xcb_map_window(g_x_connection, g_x_window);

    return 0;
}

s8 cc_x_platformGetShouldWindowClose(void) { return 0; }

s8 cc_x_platformDeinit(void) {
    xcb_disconnect(g_x_connection);

    return 0;
}
