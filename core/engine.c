// this again?
#define _POSIX_C_SOURCE 199309L

#include "engine.h"

#include <core/platform/platform.h>
#include <core/macros/macros.h>
#include <core/renderer/renderer.h>
#include <core/input/input.h>
#include <core/renderer/renderer_gl_loader.h>

// bro, i thought we would have vulkan!
#include <GL/gl.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

f64 cc_time;
f64 cc_delta;

f32 cc_real_width;
f32 cc_real_height;
f32 cc_width;
f32 cc_height;
f32 last_width;
f32 last_height;


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

u32 ccp_fbo;
u32 ccp_tex;
u32 ccp_depth;
u32 ccp_dummy_vao;
u32 ccp_prog;

s32 cc_engineMain(
    const char* title,
    s32 width, s32 height,
    void (*init)(void),
    void (*update)(void),
    void (*render)(void),
    void (*clean)(void)
) {
    CCclientState* state = cc_platformInit(CC_API_OPENGL, title, width,height);
    void* rstate = cc_rendererInit(CC_API_OPENGL, title);

    init();

    f64 starttime = getTime();


    cc_width = width;
    cc_height = height;


    glGenFramebuffers(1, &ccp_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ccp_fbo);

    glGenTextures(1, &ccp_tex);
    glBindTexture(GL_TEXTURE_2D, ccp_tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cc_width,cc_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ccp_tex, 0);

    glGenRenderbuffers(1, &ccp_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, ccp_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cc_width, cc_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ccp_depth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("framebuffer is not complete!\n");
        exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &ccp_dummy_vao);

    ccp_prog = cc_gl_loadProgram("data/eng/buf.vert", "data/eng/buf.frag");


    glBindFramebuffer(GL_FRAMEBUFFER, ccp_fbo);
    glViewport(0,0,cc_width,cc_height);

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while(!cc_platformShouldClose(state)) {
        f64 ltime = cc_time;
        cc_time = getTime() - starttime;
        cc_delta = cc_time - ltime;

        cc_inputPoll();
        
        cc_real_width = state->width;
        cc_real_height = state->height;

        update();

        if (cc_real_width != cc_width || cc_real_height != cc_height) {
            if (cc_width != last_width || cc_height != last_height) {
                glBindTexture(GL_TEXTURE_2D, ccp_tex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cc_width, cc_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindRenderbuffer(GL_RENDERBUFFER, ccp_depth);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cc_width, cc_height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, ccp_fbo);
        }

        glViewport(0,0,cc_width,cc_height);

        //glClearColor(1,0,1,1);
        //glClear(GL_COLOR_BUFFER_BIT);

        cc_rendererUpdate(rstate, cc_width, cc_height);

        cc_rendererResetTransform();
        cc_rendererResetSpriteStackCamera();

        render();

        cc_rendererFlush(rstate);

        if (cc_real_width != cc_width || cc_real_height != cc_height) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0,0,cc_real_width,cc_real_height);

            glUseProgram(ccp_prog);
            glBindVertexArray(ccp_dummy_vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ccp_tex);
            glDrawArrays(GL_TRIANGLES, 0,6);
        }

        last_width = cc_width;
        last_height = cc_height;

        cc_platformSwapBuffer(state);
    }

    clean();

    glDeleteRenderbuffers(1, &ccp_depth);
    glDeleteTextures(1, &ccp_tex);
    glDeleteFramebuffers(1, &ccp_fbo);

    cc_inputDeinit();
    cc_rendererDeinit(rstate);
    cc_platformDeinit(state);

    return 0;
}
