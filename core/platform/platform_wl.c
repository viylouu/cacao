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

struct wl_display* waylandDisplay;
struct wl_registry* waylandRegistry;
struct xdg_surface* xdgSurface;
struct wl_callback* waylandCallback;

struct wl_compositor* waylandCompositor;
struct wl_surface* waylandSurface;
struct wl_buffer* waylandBuffer;
struct wl_shm* waylandSharedMemory; //shared memory
struct xdg_wm_base* xdgShell;
struct xdg_toplevel* xdgTopLevel;
uint8_t* waylandPixel;

uint16_t width = 200;
uint16_t height = 100;

uint8_t c;

// allocate shared memory
int32_t wlAllocSharedMemory(uint64_t size) {
    int8_t name[8];
    name[0] = '/';
    name[7] = 0;
    for(uint8_t i = 1; i < 6; ++i)
        name[i] = (rand() & 23) + 97;
    
    int32_t filedesc = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR | S_IWOTH | S_IROTH);
    shm_unlink(name);
    ftruncate(filedesc, size);

    return filedesc;
}

void wlResize() {
    int32_t filedesc = wlAllocSharedMemory(width * height * 4); // width * height, * 4 because each pix is 4 bytes (rgba)
    
    waylandPixel = mmap(0, width*height*4, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, 0);

    struct wl_shm_pool* pool = wl_shm_create_pool(waylandSharedMemory, filedesc, width*height*4);

    waylandBuffer = wl_shm_pool_create_buffer(pool, 0, width,height, width*4, WL_SHM_FORMAT_ARGB8888);
    
    wl_shm_pool_destroy(pool);
    close(filedesc);
}

void draw() {
    memset(waylandPixel, c, width*height*4);

    wl_surface_attach(waylandSurface, waylandBuffer, 0,0);
    wl_surface_damage_buffer(waylandSurface, 0,0, width,height);
    wl_surface_commit(waylandSurface);
}

struct wl_callback_listener waylandCallbackListener;

void newFrame(void* data, struct wl_callback* waylandCallback, uint32_t callbackData) {
    wl_callback_destroy(waylandCallback);
    waylandCallback = wl_surface_frame(waylandSurface);
    wl_callback_add_listener(waylandCallback, &waylandCallbackListener, 0);

    ++c;
    draw();
}

struct wl_callback_listener waylandCallbackListener = {
    .done = newFrame
};

void xdgSurfaceConf(void* data, struct xdg_surface* xdgSurface, uint32_t serial) {
    xdg_surface_ack_configure(xdgSurface, serial);
    if (!waylandPixel)
        wlResize();

    draw();
}

struct xdg_surface_listener xdgSurfaceListener = {
    .configure = xdgSurfaceConf
};

void xdgTopLevelConf(void* data, struct xdg_toplevel* xdgTopLevel, int32_t width, int32_t height, struct wl_array* wlstates) {
    
}

void xdgTopLevelClose(void* data, struct xdg_toplevel* xdgTopLevel) {

}

struct xdg_toplevel_listener xdgTopLevelListener = {
    .configure = xdgTopLevelConf,
    .close = xdgTopLevelClose
};

void xdgShellPing(void* data, struct xdg_wm_base* xdgShell, uint32_t serial) {
    xdg_wm_base_pong(xdgShell, serial);
}

struct xdg_wm_base_listener xdgShellListener = {
    .ping = xdgShellPing
};

void wlRegGlob(void* data, struct wl_registry* reg, uint32_t name, const char* intf, uint32_t ver) {
    if (!strcmp(intf, wl_compositor_interface.name))
        waylandCompositor = wl_registry_bind(waylandRegistry, name, &wl_compositor_interface, 4);
    else if(!strcmp(intf, wl_shm_interface.name))
        waylandSharedMemory = wl_registry_bind(waylandRegistry, name, &wl_shm_interface, 1);
    else if(!strcmp(intf, xdg_wm_base_interface.name)) {
        xdgShell = wl_registry_bind(waylandRegistry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(xdgShell, &xdgShellListener, 0);
    }
}

void wlRegGlobRemove(void* data, struct wl_registry* reg, uint32_t name) {
    
}

struct wl_registry_listener waylandRegistryListener = {
    .global = wlRegGlob,
    .global_remove = wlRegGlobRemove
};

int8_t wlPlatformInit(const char* title, int32_t targwidth, int32_t targheight) {
    width = targwidth;
    height = targheight;

    waylandDisplay = wl_display_connect(0);
    waylandRegistry = wl_display_get_registry(waylandDisplay);

    wl_registry_add_listener(waylandRegistry, &waylandRegistryListener, 0);
    wl_display_roundtrip(waylandDisplay);

    waylandSurface = wl_compositor_create_surface(waylandCompositor);
    waylandCallback = wl_surface_frame(waylandSurface);
    wl_callback_add_listener(waylandCallback, &waylandCallbackListener, 0);

    xdgSurface = xdg_wm_base_get_xdg_surface(xdgShell, waylandSurface);
    xdg_surface_add_listener(xdgSurface, &xdgSurfaceListener, 0);

    xdgTopLevel = xdg_surface_get_toplevel(xdgSurface);
    xdg_toplevel_add_listener(xdgTopLevel, &xdgTopLevelListener, 0);

    xdg_toplevel_set_title(xdgTopLevel, title);

    wl_surface_commit(waylandSurface);

    while(wl_display_dispatch(waylandDisplay));

    return 0;
}

int8_t wlPlatformDeinit() {
    if (waylandBuffer)
        wl_buffer_destroy(waylandBuffer);

    xdg_toplevel_destroy(xdgTopLevel);
    xdg_surface_destroy(xdgSurface);

    wl_surface_destroy(waylandSurface);
    wl_display_disconnect(waylandDisplay);
    return 0;
}
