#define _POSIX_C_SOURCE 200112L

#include "platform.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <wayland-client.h>
#include <xdg-shell/xdg-shell-client-protocol.h>

typedef struct {
    CCclientState cc; // must be at start

    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_shm* shared_mem;
    struct wl_surface* surface;
    
    struct xdg_wm_base* xdg_shell;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;

    f32 offset;
    u32 last_frame;
} WLclientState;


//
// SHARED MEM
//


static void wl_randName(char* buf) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    s64 r = ts.tv_nsec;
    for (s32 i = 0; i < 6; ++i) {
        buf[i] = 'A'+(r&15)+(r&16)&2;
        r >>= 5;
    }
}

static s32 wl_createSharedMemFile(void) {
    s32 retries = 100;
    do {
        char name[] = "/wl_shm-XXXXXX";
        wl_randName(name + sizeof(name) - 7);
        --retries;
        s32 filedesc = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (filedesc >= 0) {
            shm_unlink(name);
            return filedesc;
        }
    } while(retries > 0 && errno == EEXIST);
    return -1;
}

s32 wl_allocSharedMemFile(size_t size) {
    s32 filedesc = wl_createSharedMemFile();
    if (filedesc < 0)
        return -1;
    s32 ret;
    do {
        ret = ftruncate(filedesc, size);
    } while(ret < 0 && errno == EINTR);
    if (ret < 0) {
        close(filedesc);
        return -1;
    }
    return filedesc;
}


//
// BUFFER
//


static void wl_bufferRelease(void* client, struct wl_buffer* buffer) {
    (void)client;

    wl_buffer_destroy(buffer);
}

static const struct wl_buffer_listener g_wl_buffer_listener = {
    .release = wl_bufferRelease
};


//
// RENDERING
//


static struct wl_buffer* wl_drawFrame(void* client) {
    WLclientState* state = client;

    s32 filedesc = wl_allocSharedMemFile(state->cc.size);
    if (filedesc == -1)
        return NULL;

    u32* data = mmap(NULL, state->cc.size, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, 0);
    if (data == MAP_FAILED) {
        close(filedesc);
        return NULL;
    }

    struct wl_shm_pool* pool = wl_shm_create_pool(state->shared_mem, filedesc, state->cc.size);
    struct wl_buffer* buffer = wl_shm_pool_create_buffer(pool, 0, state->cc.width, state->cc.height, state->cc.stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(filedesc);

    s32 offset = (s32)state->offset % 8;
    for (s32 y = 0; y < state->cc.height; ++y) 
        for (s32 x = 0; x < state->cc.width; ++x) {
            if (((x + offset) + (y + offset) / 8 * 8) % 16 < 8)
                data[y * state->cc.width + x] = 0xFF666666;
            else
                data[y * state->cc.width + x] = 0xFFEEEEEE;
        }

    munmap(data, state->cc.size);
    wl_buffer_add_listener(buffer, &g_wl_buffer_listener, NULL);
    return buffer;
}


//
// XDG SURFACE
//


static void xdg_surfaceConfigure(void* client, struct xdg_surface* xdg_surface, u32 serial) {
    WLclientState* state = client;

    xdg_surface_ack_configure(xdg_surface, serial);

    struct wl_buffer* buffer = wl_drawFrame(state);
    wl_surface_attach(state->surface, buffer, 0,0);
    wl_surface_commit(state->surface);
}

static const struct xdg_surface_listener g_xdg_surface_listener = {
    .configure = xdg_surfaceConfigure
};


//
// XDG SHELL
//


static void xdg_wmBasePing(void* client, struct xdg_wm_base* xdg_shell, u32 serial) {
    (void)client;

    xdg_wm_base_pong(xdg_shell, serial);
}

static const struct xdg_wm_base_listener g_xdg_shell_listener = {
    .ping = xdg_wmBasePing
};


//
// SURFACE FRAME
//


static const struct wl_callback_listener g_wl_surface_frame_listener;

static void wl_surfaceFrameDone(void* client, struct wl_callback* callback, u32 time) {
    wl_callback_destroy(callback);

    WLclientState* state = client;

    callback = wl_surface_frame(state->surface);
    wl_callback_add_listener(callback, &g_wl_surface_frame_listener, state);

    if (state->last_frame != 0) {
        s32 elapsed = time - state->last_frame;
        state->offset += elapsed / 1000.f * 24;
    }

    struct wl_buffer* buffer = wl_drawFrame(state);
    wl_surface_attach(state->surface, buffer, 0,0);
    wl_surface_damage_buffer(state->surface, 0,0, INT32_MAX, INT32_MAX);
    wl_surface_commit(state->surface);

    state->last_frame = time;
}

static const struct wl_callback_listener g_wl_surface_frame_listener = {
    .done = wl_surfaceFrameDone
};


//
// REGISTRY
//


static void wl_registryHandleGlobal(void* client, struct wl_registry* registry, u32 name, const char* interface, u32 version) {
    (void)version;

    WLclientState* state = client;

    if (!strcmp(interface, wl_compositor_interface.name))
        state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    if (!strcmp(interface, wl_shm_interface.name))
        state->shared_mem = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    if (!strcmp(interface, xdg_wm_base_interface.name)) {
        state->xdg_shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_shell, &g_xdg_shell_listener, state);
    }
}

static void wl_registryHandleGlobalRemove(void* client, struct wl_registry* registry, u32 name) {
    (void)client;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener g_wl_registry_listener = {
    .global = wl_registryHandleGlobal,
    .global_remove = wl_registryHandleGlobalRemove
};


//
// MAIN
//


void* cc_wl_platformInit(const char* title, s32 targwidth, s32 targheight) {
    WLclientState* state = malloc(sizeof(WLclientState));

    state->display = wl_display_connect(NULL);
    state->registry = wl_display_get_registry(state->display);
    wl_registry_add_listener(state->registry, &g_wl_registry_listener, state);
    wl_display_roundtrip(state->display);

    state->surface = wl_compositor_create_surface(state->compositor);
    state->xdg_surface = xdg_wm_base_get_xdg_surface(state->xdg_shell, state->surface);
    xdg_surface_add_listener(state->xdg_surface, &g_xdg_surface_listener, state);

    state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
    xdg_toplevel_set_title(state->xdg_toplevel, title);

    state->cc.width = targwidth;
    state->cc.height = targheight;
    state->cc.stride = state->cc.width * 4; 
    state->cc.pool_size = state->cc.stride * state->cc.height * 2;
    state->cc.size = state->cc.pool_size / 2;
    state->cc.running = 1;

    wl_surface_commit(state->surface);

    struct wl_callback* callback = wl_surface_frame(state->surface);
    wl_callback_add_listener(callback, &g_wl_surface_frame_listener, state);


    return state;
}

s8 cc_wl_platformIsRunning(void* client) {
    WLclientState* state = client;
    return wl_display_dispatch(state->display) != -1 && state->cc.running;
}

s8 cc_wl_platformDeinit(void* client) {
    WLclientState* state = (WLclientState*)client;

    wl_display_disconnect(state->display);

    return 0;
}

