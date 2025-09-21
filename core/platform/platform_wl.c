#define _POSIX_C_SOURCE 200112L

// resources used
// https://wayland-book.com/
// https://amini-allight.org/post/using-wayland-with-vulkan
// https://github.com/joone/opengl-wayland/blob/master/simple-egl/simple-egl.c

#include "platform.h"
#include <core/input/input.h>

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

#include <wayland-egl.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

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
    struct wl_surface* surface;
    struct wl_seat* seat;
    struct wl_keyboard* keyboard;
    struct wl_pointer* pointer;
    
    struct xdg_wm_base* xdg_shell;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;

    WLpointerEvent pointer_event;

    struct xkb_state* xkb_state;
    struct xkb_context* xkb_context;
    struct xkb_keymap* xkb_keymap;

    CCrendererApi api;
    struct {
        EGLDisplay display;
        EGLContext context;
        EGLConfig config;
        EGLSurface surface;
        struct wl_egl_window* window;

        b8 configured;
    } egl;
    
} WLclientState;


//
// XDG SURFACE
//


static void xdg_surfaceConfigure(void* client, struct xdg_surface* xdg_surface, u32 serial) {
    WLclientState* state = client;

    xdg_surface_ack_configure(xdg_surface, serial);

    switch (state->api) {
        case CC_API_VULKAN: break; // see below below
        case CC_API_OPENGL:
            if (!state->egl.configured) {
                eglSwapBuffers(state->egl.display, state->egl.surface);
                wl_surface_commit(state->surface);
                state->egl.configured = 1;
            }
            break;
    }
}

static const struct xdg_surface_listener g_xdg_surface_listener = {
    .configure = xdg_surfaceConfigure
};


//
// XDG TOPLEVEL
//


static void xdg_toplevelConfigure(void* client, struct xdg_toplevel* toplevel, s32 width, s32 height, struct wl_array* states) {
    // FINALLY ITS states INSTEAD OF state
    // YESSSSSSSSSSSS
    
    (void)toplevel;
    (void)states;

    WLclientState* state = client;

    if (!width || !height)
        return;

    state->cc.width = width;
    state->cc.height = height;

    switch (state->api) {
        case CC_API_VULKAN: break; // see below
        case CC_API_OPENGL:
            wl_egl_window_resize(state->egl.window, width, height, 0,0);
            glViewport(0,0, width, height);
            break;
    }
}

static void xdg_toplevelClose(void* client, struct xdg_toplevel* toplevel) {
    (void)toplevel;
    
    WLclientState* state = client;

    state->cc.running = 0;
}

static const struct xdg_toplevel_listener g_xdg_toplevel_listener = {
    .configure = xdg_toplevelConfigure,
    .close = xdg_toplevelClose
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
    (void)pointer;

    WLclientState* state = client;
    WLpointerEvent* event = &state->pointer_event;
    //printf("pointer frame @ %d: ", event->time);

    if (event->event_mask & POINTER_EVENT_ENTER) {
        /*printf("entered %f, %f",
                wl_fixed_to_double(event->surface_x),
                wl_fixed_to_double(event->surface_y)
            );*/
        cc_mouse_x = wl_fixed_to_double(event->surface_x);
        cc_mouse_y = wl_fixed_to_double(event->surface_y);
    }

    if (event->event_mask & POINTER_EVENT_LEAVE) {
        //printf("leave");
    }

    if (event->event_mask & POINTER_EVENT_MOTION) {
        /*printf("motion %f, %f ",
                wl_fixed_to_double(event->surface_x),
                wl_fixed_to_double(event->surface_y));*/
        cc_mouse_x = wl_fixed_to_double(event->surface_x);
        cc_mouse_y = wl_fixed_to_double(event->surface_y);
    }

    if (event->event_mask & POINTER_EVENT_BUTTON) {
        //char* stateSTOPFUCKINGNAMINGTHINGSSTATE = event->state == WL_POINTER_BUTTON_STATE_RELEASED ? "released" : "pressed";
        //printf("button %d %s ", event->button, stateSTOPFUCKINGNAMINGTHINGSSTATE);

        // yay no more state variable!!!!!!!!!!!!
        cc_mouse_buttons[event->button - CC_MOUSE_LEFT] = event->state == WL_POINTER_BUTTON_STATE_RELEASED;
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

    //printf("\n");
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
    (void)keyboard;

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
    
    (void)client;
    (void)keyboard;
    (void)serial;
    (void)serial;
    (void)surface;
    (void)kys;

    //WLclientState* state = client;

    //printf("keeb enter; pressed keys:\n");
    /*u32* key;
    wl_array_for_each(key, kys) {
        char buf[128];
        xkb_keysym_t sym = xkb_state_key_get_one_sym(state->xkb_state, *key + 8);
        xkb_keysym_get_name(sym, buf, sizeof(buf));
        printf("sym: %-12s (%d), ", buf, sym);
        xkb_state_key_get_utf8(state->xkb_state, *key + 8, buf, sizeof(buf));
        printf("utf8: '%s'\n", buf);
    }*/
}

static void wl_keyboardKey(void* client, struct wl_keyboard* keyboard, u32 cereal, u32 queTiempoHaceHoyHaceMalTiempo, u32 key, u32 stateAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA) {
    //(void)client;
    (void)keyboard;
    (void)cereal;
    (void)queTiempoHaceHoyHaceMalTiempo;

    WLclientState* state = client;
    
    //char buf[128];
    u32 keycode = key + 8;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(state->xkb_state, keycode);
    //xkb_keysym_get_name(sym, buf, sizeof(buf));
    //const char* action = stateAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA == WL_KEYBOARD_KEY_STATE_PRESSED ? "press" : "release";
    //printf("key %s: sym: %-12s (%d), ", action, buf, sym);
    //xkb_state_key_get_utf8(state->xkb_state, keycode, buf, sizeof(buf));
    //printf("utf8: '%s'\n", buf);

    cc_addDirtyKey(sym);
    cc_keyboard_keys[sym] = stateAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA == WL_KEYBOARD_KEY_STATE_PRESSED;
}

static void wl_keyboardLeave(void* client, struct wl_keyboard* keyboard, u32 serial, struct wl_surface* surface) {
    // smash
    (void)client;
    (void)keyboard;
    (void)serial;
    (void)surface;
    
    //printf("keyboard leave\n");
}

static void wl_keyboardModifiers(void* client, struct wl_keyboard* keyboard, u32 serial, u32 depressed, u32 latched, u32 locked, u32 group/*, u32 fuck, u32 this, u32 shit, u32 im, u32 out*/) {
    (void)keyboard;
    (void)serial;
    (void)client;
    (void)depressed;
    (void)latched;
    (void)locked;
    (void)group;

    // apparently the compiler doesent like it when i do that
    // (void)fuck;
    // (void)this;
    // (void)shit;
    // (void)im;
    // (void)out;

    //WLclientState* state = client;
    //xkb_state_update_mask(state->xkb_state, depressed, latched, locked, 0,0, group);
    

    // fuck this i need shift locked
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
    (void)name;

    //printf("seat name: %s\n", name);
}

static const struct wl_seat_listener g_wl_seat_listener = {
    .capabilities = wl_seatCapabilities,
    .name = wl_seatName
};


//
// REGISTRY
//


static void wl_registryGlobal(void* client, struct wl_registry* registry, u32 name, const char* interface, u32 version) {
    (void)version;

    WLclientState* state = client;

    if (!strcmp(interface, wl_compositor_interface.name))
        state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    //else if (!strcmp(interface, wl_shm_interface.name))
    //    state->shared_mem = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    else if (!strcmp(interface, xdg_wm_base_interface.name)) {
        state->xdg_shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_shell, &g_xdg_shell_listener, state);
    } else if (!strcmp(interface, wl_seat_interface.name)) {
        state->seat = wl_registry_bind(registry, name, &wl_seat_interface, 7);
        wl_seat_add_listener(state->seat, &g_wl_seat_listener, state);
    }
}

static void wl_registryGlobalRemove(void* client, struct wl_registry* registry, u32 name) {
    (void)client;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener g_wl_registry_listener = {
    .global = wl_registryGlobal,
    .global_remove = wl_registryGlobalRemove
};


//
// GL/EGL
//


static void egl_init(WLclientState* state) {
    static const EGLint contextattributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 3,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE
    };

    EGLint configattributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 1,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint major, minor, n;
    EGLBoolean ret;

    state->egl.display = eglGetDisplay(state->display);
    if (!state->egl.display) {
        printf("failed to get egl display!\n");
        exit(1);
    }

    ret = eglInitialize(state->egl.display, &major, &minor);
    if (!ret) {
        printf("failed to initialize egl!\n");
        exit(1);
    }

    ret = eglBindAPI(EGL_OPENGL_API);
    if (!ret) {
        printf("failed to bind gl|es api!\n");
        exit(1);
    }

    ret = eglChooseConfig(state->egl.display, configattributes, &state->egl.config, 1, &n);
    if (!ret || n != 1) {
        printf("failed to choose egl config!\n");
        exit(1);
    }

    state->egl.context = eglCreateContext(state->egl.display, state->egl.config, EGL_NO_CONTEXT, contextattributes);
    if (!state->egl.context) {
        printf("failed to create egl context!\n");
        exit(1);
    }
}


//
// MAIN
//


void* cc_wl_platformInit(CCrendererApi api, const char* title, s32 targwidth, s32 targheight) {
    WLclientState* state = malloc(sizeof(WLclientState));

    state->cc.width = targwidth;
    state->cc.height = targheight;
    state->cc.stride = state->cc.width * 4; 
    state->cc.pool_size = state->cc.stride * state->cc.height * 2;
    state->cc.size = state->cc.pool_size / 2;
    state->cc.running = 1;

    state->api = api;

    state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    state->display = wl_display_connect(NULL);
    state->registry = wl_display_get_registry(state->display);
    wl_registry_add_listener(state->registry, &g_wl_registry_listener, state);
    wl_display_roundtrip(state->display);

    switch(api) {
        case CC_API_VULKAN: break; // if you got here, then...
                                   // 
                                   // uhh...
                                   // 
                                   // 
                                   // 
                                   // what the fuck?
        case CC_API_OPENGL:
            egl_init(state);
            break;
    }

    state->surface = wl_compositor_create_surface(state->compositor);
    state->xdg_surface = xdg_wm_base_get_xdg_surface(state->xdg_shell, state->surface);
    xdg_surface_add_listener(state->xdg_surface, &g_xdg_surface_listener, state);

    state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
    xdg_toplevel_add_listener(state->xdg_toplevel, &g_xdg_toplevel_listener, state);
    xdg_toplevel_set_title(state->xdg_toplevel, title);

    switch (api) {
        case CC_API_VULKAN: break; // see above
        case CC_API_OPENGL:
            state->egl.window = wl_egl_window_create(state->surface, state->cc.width, state->cc.height);
            state->egl.surface = eglCreateWindowSurface(state->egl.display, state->egl.config, state->egl.window, NULL);
            eglMakeCurrent(state->egl.display, state->egl.surface, state->egl.surface, state->egl.context);
            eglSwapInterval(state->egl.display, 0);
            break;
    }

    wl_surface_commit(state->surface);
    wl_display_roundtrip(state->display);
    wl_surface_commit(state->surface);

    xkb_state_update_mask(state->xkb_state, (1 << xkb_keymap_mod_get_index(state->xkb_keymap, "Shift")), 0,0,0,0,0);

    return state;
}

s8 cc_wl_platformIsRunning(void* client) {
    WLclientState* state = client;
    return wl_display_dispatch(state->display) != -1 && state->cc.running;
}

void cc_wl_platformDeinit(void* client) {
    WLclientState* state = client;

    switch (state->api) {
        case CC_API_VULKAN: break; // see above above
        case CC_API_OPENGL:
            eglTerminate(state->egl.display);
            eglReleaseThread();
            break;
    }

    wl_seat_destroy(state->seat);
    xdg_toplevel_destroy(state->xdg_toplevel);
    xdg_surface_destroy(state->xdg_surface);
    wl_surface_destroy(state->surface);
    xdg_wm_base_destroy(state->xdg_shell);
    wl_compositor_destroy(state->compositor);
    wl_registry_destroy(state->registry);
    wl_display_disconnect(state->display);

    free(state);
}

void cc_wl_platformSwapBuffers(void* client) {
    WLclientState* state = client;

    eglSwapBuffers(state->egl.display, state->egl.surface);
}
