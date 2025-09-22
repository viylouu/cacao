// this again?
#define _POSIX_C_SOURCE 199309L

#include "engine.h"

#include <core/platform/platform.h>
#include <core/macros/macros.h>
#include <core/renderer/renderer.h>
#include <core/input/input.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

f64 cc_time;
f64 cc_delta;

f64 getTime(void) {
#ifdef _WIN32
    static LARGE_INTEGER freq = {0};
    if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (f64)counter.QuadPart / (f64)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
#endif
}

s32 cc_engineMain(
    const char* title,
    s32 width, s32 height,
    void (*init)(void),
    void (*update)(void),
    void (*render)(void),
    void (*clean)(void)
) {
    void* state = cc_platformInit(CC_API_OPENGL, title, width,height);
    void* rstate = cc_rendererInit(CC_API_OPENGL, title);

    init();

    f64 starttime = getTime();

    while(!cc_platformShouldClose(state)) {
        f64 ltime = cc_time;
        cc_time = getTime() - starttime;
        cc_delta = cc_time - ltime;

        cc_inputPoll();
        
        cc_rendererUpdate(rstate, state);

        update();
        render();

        cc_rendererFlush(rstate);

        cc_platformSwapBuffer(state);
    }

    clean();

    cc_inputDeinit();
    cc_rendererDeinit(rstate);
    cc_platformDeinit(state);

    return 0;
}
