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

struct wl_display* wldisp;
struct wl_registry* wlreg;
struct xdg_surface* xdgsurf;

struct wl_compositor* wlcomp;
struct wl_surface* wlsurf;
struct wl_buffer* wlbuf;
struct wl_shm* wlshm; //shared memory
struct xdg_wm_base* xdgshell;
struct xdg_toplevel* xdgtop;
uint8_t* wlpix;

uint16_t width = 200;
uint16_t height = 100;

// allocate shared memory
int32_t wlAllocShm(uint64_t size) {
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
    int32_t filedesc = wlAllocShm(width * height * 4); // width * height, * 4 because each pix is 4 bytes (rgba)
    
    wlpix = mmap(0, width*height*4, PROT_READ | PROT_WRITE, MAP_SHARED, filedesc, 0);

    struct wl_shm_pool* pool = wl_shm_create_pool(wlshm, filedesc, width*height*4);

    wlbuf = wl_shm_pool_create_buffer(pool, 0, width,height, width*4, WL_SHM_FORMAT_ARGB8888);
    
    wl_shm_pool_destroy(pool);
    close(filedesc);
}

void draw() {
    
}

void xdgsurfConf(void* data, struct xdg_surface* xdgsurf, uint32_t serial) {
    xdg_surface_ack_configure(xdgsurf, serial);
    if (!wlpix)
        wlResize();

    draw();

    wl_surface_attach(wlsurf, wlbuf, 0,0);
    wl_surface_damage_buffer(wlsurf, 0,0, width,height);
    wl_surface_commit(wlsurf);
}

struct xdg_surface_listener xdgsurflist = {
    .configure = xdgsurfConf
};

void xdgtopConf(void* data, struct xdg_toplevel* xdgtop, int32_t width, int32_t height, struct wl_array* wlstates) {
    
}

void xdgtopClose(void* data, struct xdg_toplevel* xdgtop) {

}

struct xdg_toplevel_listener xdgtoplist = {
    .configure = xdgtopConf,
    .close = xdgtopClose
};

void xdgshellPing(void* data, struct xdg_wm_base* xdgshell, uint32_t serial) {
    xdg_wm_base_pong(xdgshell, serial);
}

struct xdg_wm_base_listener xdgshelllist = {
    .ping = xdgshellPing
};

void wlRegGlob(void* data, struct wl_registry* reg, uint32_t name, const char* intf, uint32_t ver) {
    if (!strcmp(intf, wl_compositor_interface.name))
        wlcomp = wl_registry_bind(wlreg, name, &wl_compositor_interface, 4);
    else if(!strcmp(intf, wl_shm_interface.name))
        wlshm = wl_registry_bind(wlreg, name, &wl_shm_interface, 1);
    else if(!strcmp(intf, xdg_wm_base_interface.name)) {
        xdgshell = wl_registry_bind(wlreg, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(xdgshell, &xdgshelllist, 0);
    }
}

void wlRegGlobRemove(void* data, struct wl_registry* reg, uint32_t name) {
    
}

struct wl_registry_listener wlreglist = {
    .global = wlRegGlob,
    .global_remove = wlRegGlobRemove
};

int8_t wlPlatformInit(const char* title, int32_t targwidth, int32_t targheight) {
    width = targwidth;
    height = targheight;

    wldisp = wl_display_connect(0);
    wlreg = wl_display_get_registry(wldisp);

    wl_registry_add_listener(wlreg, &wlreglist, 0);
    wl_display_roundtrip(wldisp);

    wlsurf = wl_compositor_create_surface(wlcomp);

    xdgsurf = xdg_wm_base_get_xdg_surface(xdgshell, wlsurf);
    xdg_surface_add_listener(xdgsurf, &xdgsurflist, 0);

    xdgtop = xdg_surface_get_toplevel(xdgsurf);
    xdg_toplevel_add_listener(xdgtop, &xdgtoplist, 0);

    xdg_toplevel_set_title(xdgtop, title);

    wl_surface_commit(wlsurf);

    while(wl_display_dispatch(wldisp));

    return 0;
}

int8_t wlPlatformDeinit() {
    if (wlbuf)
        wl_buffer_destroy(wlbuf);

    wl_surface_destroy(wlsurf);
    wl_display_disconnect(wldisp);
    return 0;
}
