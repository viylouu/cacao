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
#include <xkbcommon/xkbcommon.h>

typedef enum {
    POINTER_EVENT_ENTER         = 1 << 0,
    POINTER_EVENT_LEAVE         = 1 << 1,
    POINTER_EVENT_MOTION        = 1 << 2,
    POINTER_EVENT_BUTTON        = 1 << 3,
    POINTER_EVENT_AXIS          = 1 << 4,
    POINTER_EVENT_AXIS_SOURCE   = 1 << 5,
    POINTER_EVENT_AXIS_STOP     = 1 << 6,
    POINTER_EVENT_AXIS_DISCRETE = 1 << 7
} WLpointerEventMask;

typedef struct {
    u32 event_mask;
    wl_fixed_t surface_x;
    wl_fixed_t surface_y;
    u32 button;
    u32 state;
    u32 time;
    u32 serial;
    struct {
        u8 valid;
        wl_fixed_t value;
        s32 discrete;
    } axes[2];
    u32 axis_source;
} WLpointerEvent;

typedef struct {
    CCclientState cc; // must be at start

    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_shm* shared_mem;
    struct wl_surface* surface;
    struct wl_seat* seat;
    struct wl_keyboard* keyboard;
    struct wl_pointer* pointer;
    
    struct xdg_wm_base* xdg_shell;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;

    f32 offset;
    u32 last_frame;

    WLpointerEvent pointer_event;

    struct xkb_state* xkb_state;
    struct xkb_context* xkb_context;
    struct xkb_keymap* xkb_keymap;
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
// POINTER
//


// wouldnt it just be reaaaally fucking funny if this were actually server code because of how much shit there is?

static void wl_pointerEnter(void* client, struct wl_pointer* pointer, u32 serial, struct wl_surface* surface, wl_fixed_t surfacex, wl_fixed_t surfacey) {
    (void)pointer;
    (void)surface;
    
    WLclientState* state = client;

    state->pointer_event.event_mask |= POINTER_EVENT_ENTER;
    state->pointer_event.serial = serial;
    state->pointer_event.surface_x = surfacex;
    state->pointer_event.surface_y = surfacey;
}

static void wl_pointerLeave(void* client, struct wl_pointer* pointer, u32 serial, struct wl_surface* surface) {
    (void)pointer;
    (void)surface;

    WLclientState* state = client;

    state->pointer_event.serial = serial;
    state->pointer_event.event_mask |= POINTER_EVENT_LEAVE;
}

static void wl_pointerMotion(void* client, struct wl_pointer* pointer, u32 time, wl_fixed_t surfacex, wl_fixed_t surfacey) {
    (void)pointer;

    WLclientState* state = client;

    state->pointer_event.event_mask |= POINTER_EVENT_MOTION;
    state->pointer_event.time = time;
    state->pointer_event.surface_x = surfacex;
    state->pointer_event.surface_y = surfacey;
}

static void wl_pointerButton(void* client, struct wl_pointer* pointer, u32 serial, u32 time, u32 button, u32 stateAAAAA) {
    (void)pointer;

    WLclientState* state = client;

    state->pointer_event.event_mask |= POINTER_EVENT_BUTTON;
    state->pointer_event.time = time;
    state->pointer_event.serial = serial;
    state->pointer_event.button = button;
    state->pointer_event.state = stateAAAAA; // uniqueness silly willy
                                             // can you tell i'm tired
}

static void wl_pointerAxis(void* client, struct wl_pointer* pointer, u32 time, u32 axis, wl_fixed_t value) {
    (void)pointer;

    WLclientState* state = client;

    state->pointer_event.event_mask |= POINTER_EVENT_AXIS;
    state->pointer_event.time = time;
    state->pointer_event.axes[axis].valid = 1;
    state->pointer_event.axes[axis].value = value;
}

static void wl_pointerAxisSource(void* client, struct wl_pointer* pointer, u32 axissource) {
    (void)pointer;

    WLclientState* state = client;

    state->pointer_event.event_mask |= POINTER_EVENT_AXIS_SOURCE;
    state->pointer_event.axis_source = axissource;
}

static void wl_pointerAxisStop(void* client, struct wl_pointer* pointer, u32 time, u32 axis) {
    (void)pointer;

    WLclientState* state = client;

    state->pointer_event.time = time;
    state->pointer_event.event_mask |= POINTER_EVENT_AXIS_STOP;
    state->pointer_event.axes[axis].valid = 1;
}

static void wl_pointerAxisDiscrete(void* client, struct wl_pointer* pointer, u32 axis, s32 discrete) {
    (void)pointer;

    WLclientState* state = client;

    state->pointer_event.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
    state->pointer_event.axes[axis].valid = 1;
    state->pointer_event.axes[axis].discrete = discrete;
}

static void wl_pointerFrame(void* client, struct wl_pointer* pointer) {
    WLclientState* state = client;
    WLpointerEvent* event = &state->pointer_event;
    printf("pointer frame @ %d: ", event->time);

    if (event->event_mask & POINTER_EVENT_ENTER) {
        printf("entered %f, %f",
                wl_fixed_to_double(event->surface_x),
                wl_fixed_to_double(event->surface_y)
            );
    }

    if (event->event_mask & POINTER_EVENT_LEAVE) {
        printf("leave");
    }

    if (event->event_mask & POINTER_EVENT_MOTION) {
        printf("motion %f, %f ",
                wl_fixed_to_double(event->surface_x),
                wl_fixed_to_double(event->surface_y));
    }

    if (event->event_mask & POINTER_EVENT_BUTTON) {
        char* stateSTOPFUCKINGNAMINGTHINGSSTATE = event->state == WL_POINTER_BUTTON_STATE_RELEASED ? "released" : "pressed";
        printf("button %d %s ", event->button, stateSTOPFUCKINGNAMINGTHINGSSTATE);
    }

    u32 axis_events = POINTER_EVENT_AXIS
        | POINTER_EVENT_AXIS_SOURCE
        | POINTER_EVENT_AXIS_STOP
        | POINTER_EVENT_AXIS_DISCRETE;
    char* axis_name[2] = {
        [WL_POINTER_AXIS_VERTICAL_SCROLL] = "vertical",
        [WL_POINTER_AXIS_HORIZONTAL_SCROLL] = "horizontal"
    };
    char* axis_source[4] = {
        [WL_POINTER_AXIS_SOURCE_WHEEL] = "wheel",
        [WL_POINTER_AXIS_SOURCE_FINGER] = "finger",
        [WL_POINTER_AXIS_SOURCE_CONTINUOUS] = "continuous",
        [WL_POINTER_AXIS_SOURCE_WHEEL_TILT] = "wheel tilt"
    };

    if (event->event_mask & axis_events)
        for (size_t i = 0; i < 2; ++i) {
            if (!event->axes[i].valid)
                continue;
            printf("%s axis ", axis_name[i]);
            if (event->event_mask & POINTER_EVENT_AXIS)
                printf("value %f ", wl_fixed_to_double(event->axes[i].value));
            if (event->event_mask & POINTER_EVENT_AXIS_DISCRETE)
                printf("discrete %d ", event->axes[i].discrete);
            if (event->event_mask & POINTER_EVENT_AXIS_SOURCE)
                printf("via %s ", axis_source[event->axis_source]);
            if (event->event_mask & POINTER_EVENT_AXIS_STOP)
                printf("(stopped) ");
        }

    printf("\n");
    memset(event, 0, sizeof(*event));
}

static const struct wl_pointer_listener g_wl_pointer_listener = {
    .enter = wl_pointerEnter,
    .leave = wl_pointerLeave,
    .motion = wl_pointerMotion,
    .button = wl_pointerButton,
    .axis = wl_pointerAxis,
    .frame = wl_pointerFrame,
    .axis_source = wl_pointerAxisSource,
    .axis_stop = wl_pointerAxisStop,
    .axis_discrete = wl_pointerAxisDiscrete
};


//
// KEYBOARD
//


static void wl_keyboardKeymap(void* client, struct wl_keyboard* keyboard, u32 format, s32 filedesc, u32 size) {
    WLclientState* state = client;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        printf("fix your fucking keymap bucko\n");
        exit(1);
    }

    char* mapsharedmem = mmap(0, size, PROT_READ, MAP_SHARED, filedesc, 0);
    if (mapsharedmem == MAP_FAILED) {
        printf("failed to keeb map\n");
        exit(1);
    }

    struct xkb_keymap* xkb_keymap = xkb_keymap_new_from_string(state->xkb_context, mapsharedmem, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(mapsharedmem, size);
    close(filedesc);

    struct xkb_state* xkb_state = xkb_state_new(xkb_keymap);
    xkb_keymap_unref(state->xkb_keymap);
    xkb_state_unref(state->xkb_state);
    state->xkb_keymap = xkb_keymap;
    state->xkb_state = xkb_state;
}

static void wl_keyboardEnter(void* client, struct wl_keyboard* keyboard, u32 serial, struct wl_surface* surface, struct wl_array* kys) {
    // yes i know keys is spelt wrong
    // am i going to change it?
    //
    // no
    
    (void)keyboard;
    (void)serial;
    (void)surface;

    WLclientState* state = client;

    printf("keeb enter; pressed keys:\n");
    u32* key;
    wl_array_for_each(key, kys) {
        char buf[128];
        xkb_keysym_t sym = xkb_state_key_get_one_sym(state->xkb_state, *key + 8);
        xkb_keysym_get_name(sym, buf, sizeof(buf));
        printf("sym: %-12s (%d), ", buf, sym);
        xkb_state_key_get_utf8(state->xkb_state, *key + 8, buf, sizeof(buf));
        printf("utf8: '%s'\n", buf);
    }
}

static void wl_keyboardKey(void* client, struct wl_keyboard* keyboard, u32 cereal, u32 queTiempoHaceHoyHaceMalTiempo, u32 key, u32 stateAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA) {
    (void)keyboard;
    (void)cereal;
    (void)queTiempoHaceHoyHaceMalTiempo;

    WLclientState* state = client;
    
    char buf[128];
    u32 keycode = key + 8;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(state->xkb_state, keycode);
    xkb_keysym_get_name(sym, buf, sizeof(buf));
    const char* action = stateAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release";
    printf("key %s: sym: %-12s (%d), ", action, buf, sym);
    xkb_state_key_get_utf8(state->xkb_state, keycode, buf, sizeof(buf));
    printf("utf8: '%s'\n", buf);
}

static void wl_keyboardLeave(void* client, struct wl_keyboard* keyboard, u32 serial, struct wl_surface* surface) {
    // smash
    (void)client;
    (void)keyboard;
    (void)serial;
    (void)surface;
    
    printf("keyboard leave\n");
}

static void wl_keyboardModifiers(void* client, struct wl_keyboard* keyboard, u32 serial, u32 depressed, u32 latched, u32 locked, u32 group/*, u32 fuck, u32 this, u32 shit, u32 im, u32 out*/) {
    (void)keyboard;
    (void)serial;

    // apparently the compiler doesent like it when i do that
    // (void)fuck;
    // (void)this;
    // (void)shit;
    // (void)im;
    // (void)out;

    WLclientState* state = client;
    xkb_state_update_mask(state->xkb_state, depressed, latched, locked, 0,0, group);
}

static void wl_keyboardRepeatInfo(void* client, struct wl_keyboard* keyboard, s32 rate, s32 delay) {
    // AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    // /* Left as an exercise for the reader */ MY ASS
    // NOW I HAVE TO void THIS SHIT
    // DOESENT FUCKING TELL ME HOW xkb WORKS
    // YET EXPECTS ME TO KNOW

    (void)client;
    (void)keyboard;
    (void)rate;
    (void)delay;
}

static const struct wl_keyboard_listener g_wl_keyboard_listener = {
    .keymap = wl_keyboardKeymap,
    .enter = wl_keyboardEnter,
    .leave = wl_keyboardLeave,
    .key = wl_keyboardKey,
    .modifiers = wl_keyboardModifiers,
    .repeat_info = wl_keyboardRepeatInfo
};


//
// SEAT
//


static void wl_seatCapabilities(void* client, struct wl_seat* seat, u32 capabilities) {
    (void)seat;

    WLclientState* state = client;

    b32 hascursor = capabilities & WL_SEAT_CAPABILITY_POINTER;
    
    if (hascursor && !state->pointer) {
        state->pointer = wl_seat_get_pointer(state->seat);
        wl_pointer_add_listener(state->pointer, &g_wl_pointer_listener, state);
    } else if (!hascursor && state->pointer) {
        wl_pointer_release(state->pointer);
        state->pointer = 0;
    }

    b32 haskeyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

    if (haskeyboard && !state->keyboard) {
        state->keyboard = wl_seat_get_keyboard(state->seat);
        wl_keyboard_add_listener(state->keyboard, &g_wl_keyboard_listener, state);
    } else if (!haskeyboard && state->keyboard) {
        wl_keyboard_release(state->keyboard);
        state->keyboard = 0;
    }
}

static void wl_seatName(void* client, struct wl_seat* seat, const char* name) {
    (void)client;
    (void)seat;

    printf("seat name: %s\n", name);
}

static const struct wl_seat_listener g_wl_seat_listener = {
    .capabilities = wl_seatCapabilities,
    .name = wl_seatName
};


//
// REGISTRY
//


static void wl_registryHandleGlobal(void* client, struct wl_registry* registry, u32 name, const char* interface, u32 version) {
    (void)version;

    WLclientState* state = client;

    if (!strcmp(interface, wl_compositor_interface.name))
        state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    else if (!strcmp(interface, wl_shm_interface.name))
        state->shared_mem = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    else if (!strcmp(interface, xdg_wm_base_interface.name)) {
        state->xdg_shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_shell, &g_xdg_shell_listener, state);
    } else if (!strcmp(interface, wl_seat_interface.name)) {
        state->seat = wl_registry_bind(registry, name, &wl_seat_interface, 7);
        wl_seat_add_listener(state->seat, &g_wl_seat_listener, state);
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

    state->cc.width = targwidth;
    state->cc.height = targheight;
    state->cc.stride = state->cc.width * 4; 
    state->cc.pool_size = state->cc.stride * state->cc.height * 2;
    state->cc.size = state->cc.pool_size / 2;
    state->cc.running = 1;

    state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);;

    state->display = wl_display_connect(NULL);
    state->registry = wl_display_get_registry(state->display);
    wl_registry_add_listener(state->registry, &g_wl_registry_listener, state);
    wl_display_roundtrip(state->display);

    state->surface = wl_compositor_create_surface(state->compositor);
    state->xdg_surface = xdg_wm_base_get_xdg_surface(state->xdg_shell, state->surface);
    xdg_surface_add_listener(state->xdg_surface, &g_xdg_surface_listener, state);

    state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
    xdg_toplevel_set_title(state->xdg_toplevel, title);

    

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

