#define _POSIX_C_SOURCE 200809L // bro wtf

// huge thanks to will thomas for his tutorial https://www.youtube.com/watch?v=iIVIu7YRdY0&t=3346s

#include "platform.h"

#include <wayland-client.h>
#include <xdg-shell/xdg-shell-client-protocol.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct wl_display* g_wl_display;
struct wl_registry* g_wl_registry;
struct xdg_surface* g_xdg_surface;
struct wl_callback* g_wl_callback;

struct wl_compositor* g_wl_compositor;
struct wl_surface* g_wl_surface;
struct wl_buffer* g_wl_buffer;
struct wl_shm* g_wl_shared_memory;
struct xdg_wm_base* g_xdg_shell;
struct xdg_toplevel* g_xdg_top_level;
struct wl_seat* g_wl_seat;
struct wl_keyboard* g_wl_keyboard;

uint8_t* g_wl_pixel;

uint16_t cc_wl_width = 200;
uint16_t cc_wl_height = 100;

uint8_t cc_wl_tick;

uint8_t cc_wl_shouldWindowClose;

void (*cc_wl_drawFunction)(void) = 0;

// allocate shared memory
int32_t wl_allocSharedMemory(uint64_t size) {
    char name[8];
    name[0] = '/';
    name[7] = 0;
    for(uint8_t i = 1; i < 6; ++i)
        name[i] = (rand() & 23) + 97;
    
    int32_t filedesc = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);
    shm_unlink(name);
    ftruncate(filedesc, size);

    return filedesc;
}

void wl_resize() {
    int32_t filedesc = wl_allocSharedMemory(cc_wl_width * cc_wl_height * 4); // width * height, * 4 because each pix is 4 bytes (rgba)
    
    g_wl_pixel = mmap(0, cc_wl_width * cc_wl_height * 4, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, 0);

    struct wl_shm_pool* pool = wl_shm_create_pool(g_wl_shared_memory, filedesc, cc_wl_width * cc_wl_height * 4);

    g_wl_buffer = wl_shm_pool_create_buffer(pool, 0, cc_wl_width, cc_wl_height, cc_wl_width * 4, WL_SHM_FORMAT_ARGB8888);
    
    wl_shm_pool_destroy(pool);
    close(filedesc);
}

void draw() {
    memset(g_wl_pixel, cc_wl_tick, cc_wl_width * cc_wl_height * 4);

    cc_wl_drawFunction();

    wl_surface_attach(g_wl_surface, g_wl_buffer, 0,0);
    wl_surface_damage_buffer(g_wl_surface, 0,0, cc_wl_width, cc_wl_height);
    wl_surface_commit(g_wl_surface);
}

struct wl_callback_listener g_wl_callback_listener;

void newFrame(void* data, struct wl_callback* wl_callback, uint32_t callbackdata) {
    (void)data;
    (void)callbackdata;

    wl_callback_destroy(wl_callback);
    wl_callback = wl_surface_frame(g_wl_surface);
    wl_callback_add_listener(wl_callback, &g_wl_callback_listener, 0);

    ++cc_wl_tick;
    draw();
}

struct wl_callback_listener g_wl_callback_listener = {
    .done = newFrame
};

void xdg_surfaceConf(void* data, struct xdg_surface* xdg_surface, uint32_t serial) {
    (void)data;

    xdg_surface_ack_configure(xdg_surface, serial);
    if (!g_wl_pixel)
        wl_resize();

    draw();
}

struct xdg_surface_listener g_xdg_surface_listener = {
    .configure = xdg_surfaceConf
};

void xdg_topLevelConf(void* data, struct xdg_toplevel* xdg_top_level, int32_t newwidth, int32_t newheight, struct wl_array* wl_states) {
    (void)data;
    (void)xdg_top_level;
    (void)wl_states;

    if (!newwidth && !newheight) return;

    if (newwidth != cc_wl_width || newheight != cc_wl_height) {
        munmap(g_wl_pixel, cc_wl_width * cc_wl_height * 4);
        cc_wl_width = newwidth;
        cc_wl_height = newheight;
        wl_resize();
    }
}

void xdg_topLevelClose(void* data, struct xdg_toplevel* xdg_top_level) {
    (void)data;
    (void)xdg_top_level;

    cc_wl_shouldWindowClose = 1;
}

struct xdg_toplevel_listener g_xdg_top_level_listener = {
    .configure = xdg_topLevelConf,
    .close = xdg_topLevelClose
};

void xdg_shellPing(void* data, struct xdg_wm_base* xdg_shell, uint32_t serial) {
    (void)data;

    xdg_wm_base_pong(xdg_shell, serial);
}

struct xdg_wm_base_listener g_xdg_shell_listener = {
    .ping = xdg_shellPing
};

void wl_keyboardKeymap(void* data, struct wl_keyboard* wl_keyboard, uint32_t format, int32_t filedesc, uint32_t size) {
    (void)data;
    (void)wl_keyboard;
    (void)format;
    (void)filedesc;
    (void)size;
}
void wl_keyboardEnter(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* waylandSurface, struct wl_array* keys) {
    (void)data;
    (void)wl_keyboard;
    (void)serial;
    (void)waylandSurface;
    (void)keys;
}
void wl_keyboardLeave(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, struct wl_surface* wl_surface) {
    (void)data;
    (void)wl_keyboard;
    (void)serial;
    (void)wl_surface;
}
void wl_keyboardKey(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data;
    (void)wl_keyboard;
    (void)serial;
    (void)time;
    (void)state;

    printf("%u\n", key); 
}
void wl_keyboardModifiers(void* data, struct wl_keyboard* wl_keyboard, uint32_t serial, uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group) {
    (void)data;
    (void)wl_keyboard;
    (void)serial;
    (void)depressed;
    (void)latched;
    (void)locked;
    (void)group;
}
void wl_keyboardRepeatInfo(void* data, struct wl_keyboard* wl_keyboard, int32_t rate, int32_t delay) {
    (void)data;
    (void)wl_keyboard;
    (void)rate;
    (void)delay;
}

struct wl_keyboard_listener g_wl_keyboard_listener = {
    .keymap = wl_keyboardKeymap,
    .enter = wl_keyboardEnter,
    .leave = wl_keyboardLeave,
    .key = wl_keyboardKey,
    .modifiers = wl_keyboardModifiers,
    .repeat_info = wl_keyboardRepeatInfo
};

void wl_seatCap(void* data, struct wl_seat* wl_seat, uint32_t cap) {
    (void)data;

    if (cap & WL_SEAT_CAPABILITY_KEYBOARD && !g_wl_keyboard) {
        g_wl_keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(g_wl_keyboard, &g_wl_keyboard_listener, 0);
    }
}

void wl_seatName(void* data, struct wl_seat* waylandSeat, const char* name) {
    (void)data;
    (void)waylandSeat;
    (void)name;
}

struct wl_seat_listener g_wl_seat_listener = {
    .capabilities = wl_seatCap,
    .name = wl_seatName
};

void wl_regGlob(void* data, struct wl_registry* registry, uint32_t name, const char* intf, uint32_t version) {
    (void)data;
    (void)version;

    if (!strcmp(intf, wl_compositor_interface.name))
        g_wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    else if(!strcmp(intf, wl_shm_interface.name))
        g_wl_shared_memory = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    else if(!strcmp(intf, xdg_wm_base_interface.name)) {
        g_xdg_shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(g_xdg_shell, &g_xdg_shell_listener, 0);
    }
    else if(!strcmp(intf, wl_seat_interface.name)) {
        g_wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(g_wl_seat, &g_wl_seat_listener, 0);
    }
}

void wl_regGlobRemove(void* data, struct wl_registry* registry, uint32_t name) {
    (void)data;
    (void)registry;
    (void)name;
}

struct wl_registry_listener g_wayland_registry_listener = {
    .global = wl_regGlob,
    .global_remove = wl_regGlobRemove
};

int8_t cc_wl_platformInit(const char* title, int32_t targwidth, int32_t targheight) {
    cc_wl_width = targwidth;
    cc_wl_height = targheight;

    g_wl_display = wl_display_connect(0);
    g_wl_registry = wl_display_get_registry(g_wl_display);

    wl_registry_add_listener(g_wl_registry, &g_wayland_registry_listener, 0);
    wl_display_roundtrip(g_wl_display);

    g_wl_surface = wl_compositor_create_surface(g_wl_compositor);
    g_wl_callback = wl_surface_frame(g_wl_surface);
    wl_callback_add_listener(g_wl_callback, &g_wl_callback_listener, 0);

    g_xdg_surface = xdg_wm_base_get_xdg_surface(g_xdg_shell, g_wl_surface);
    xdg_surface_add_listener(g_xdg_surface, &g_xdg_surface_listener, 0);

    g_xdg_top_level = xdg_surface_get_toplevel(g_xdg_surface);
    xdg_toplevel_add_listener(g_xdg_top_level, &g_xdg_top_level_listener, 0);

    xdg_toplevel_set_title(g_xdg_top_level, title);

    wl_surface_commit(g_wl_surface);

    return 0;
}

int8_t cc_wl_getShouldWindowClose(void) { return !wl_display_dispatch(g_wl_display) || cc_wl_shouldWindowClose; }

int8_t cc_wl_platformDeinit(void) {
    if (g_wl_keyboard)
        wl_keyboard_destroy(g_wl_keyboard);

    wl_seat_destroy(g_wl_seat);

    if (g_wl_buffer)
        wl_buffer_destroy(g_wl_buffer);

    xdg_toplevel_destroy(g_xdg_top_level);
    xdg_surface_destroy(g_xdg_surface);

    wl_surface_destroy(g_wl_surface);
    wl_display_disconnect(g_wl_display);
    return 0;
}
