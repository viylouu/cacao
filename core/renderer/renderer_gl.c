#include "renderer.h"

#include <GL/gl.h>
#include <stdio.h>
#include "renderer_gl_loader.h"
#include <core/types/mat4.h>
#include <string.h>

// table of contents:
//  TYPES
//  SHADERS
//  MAIN
//  FUNCS
//      2D
//


//
// TYPES
//

struct {
    // naming convention:
    //  its basically just whats easy for me to type
    //  1 - a
    //  2 - s
    //  3 - d
    //  4 - f
    //  etc.
    //  2.5 - sg

    struct {
        struct {
            u32 vao;
            u32 prog;
            u32 bo;
            u32 tbo;
            s32 loc_inst_size;
            s32 loc_insts;
            s32 loc_proj;
        } rect;
    } s;
} bufs;

b8 cc_renderer_use_wayland;

mat4 proj2d;

f32 r,g,b,a;

const s32 CC_GL_MAX_BATCH_SIZE = 65536;
s32 CC_GL_MAX_BUFFER_SIZE;

#pragma pack(4)
typedef struct {
    f32 x, y, w, h;
    f32 r, g, b, a;
} GLinstanceData;
#pragma pack()

typedef enum {
    CC_BATCH_NONE,
    CC_2D_BATCH_RECT
} CCbatchType;

struct {
    CCbatchType type;
    GLinstanceData* data;
    u32 data_size;
    u32 data_capac;
} batch;

//
// SHADERS
//


u32 cc_gl_compileProgram(u32* shaders, u32 amount) {
    u32 program = glCreateProgram();

    for (u32 i = 0; i < amount; ++i)
        glAttachShader(program, shaders[i]);
    glLinkProgram(program);

    s32 success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        s32 length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        char* log = malloc(length);
        glGetProgramInfoLog(program, length, 0, log);

        printf("program compile error!\n%s\n", log);
        free(log);

        glDeleteProgram(program);
        exit(1);
    }

    return program;
}

u32 cc_gl_loadShaderFromSource(u32 type, const char** source) {
    u32 shader = glCreateShader(type);
    glShaderSource(shader, 1, source, 0);
    glCompileShader(shader);

    s32 success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        s32 length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        char* log = malloc(length);
        glGetShaderInfoLog(shader, length, 0, log);

        printf("shader compile error!\n%s\n", log);
        free(log);

        glDeleteShader(shader);
        exit(1);
    }

    return shader;
}

u32 cc_gl_loadProgramFromSource(const char** vert, const char** frag) {
    u32 vertex = cc_gl_loadShaderFromSource(GL_VERTEX_SHADER, vert);
    u32 fragment = cc_gl_loadShaderFromSource(GL_FRAGMENT_SHADER, frag);

    u32 shaders[2] = { vertex, fragment };
    u32 program = cc_gl_compileProgram(shaders, 2);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

char* loadShaderSrc(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) { 
        printf("failed to open shader at \"%s\"!\n", path); 
        exit(1); 
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file); // why is it long and not long long so i can use s64? idfk
    rewind(file);

    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        printf("failed to allocate size for the shader at \"%s\"!\n", path);
        exit(1);
    }

    fread(buffer, 1, size, file);
    fclose(file);

    buffer[size] = '\0';

    return buffer;
}

u32 cc_gl_loadShader(u32 type, const char* path) {
    char* buffer = loadShaderSrc(path);
    if (!buffer) {
        printf("failed to read shader at \"%s\"!\n", path);
        exit(1);
    }

    const char* source = buffer;

    u32 shader = cc_gl_loadShaderFromSource(type, &source);

    free(buffer);

    return shader;
}

u32 cc_gl_loadProgram(const char* vert, const char* frag) {
    char* vbuf = loadShaderSrc(vert);
    char* fbuf = loadShaderSrc(frag);

    const char* vsrc = vbuf;
    const char* fsrc = fbuf;

    u32 program = cc_gl_loadProgramFromSource(&vsrc, &fsrc);

    free(vbuf);
    free(fbuf);

    return program;
}


//
// MAIN
//


void cc_gl_rendererInit(void) {
    cc_gl_load();

    batch.data = NULL;
    batch.data_size = 0;
    batch.data_capac = 0;
    batch.type = CC_BATCH_NONE;

    CC_GL_MAX_BUFFER_SIZE = CC_GL_MAX_BATCH_SIZE * sizeof(GLinstanceData);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bufs.s.rect.prog = cc_gl_loadProgram("data/eng/rect.vert", "data/eng/rect.frag");
    glGenVertexArrays(1, &bufs.s.rect.vao);

    bufs.s.rect.loc_proj      = glGetUniformLocation(bufs.s.rect.prog, "proj");
    bufs.s.rect.loc_insts     = glGetUniformLocation(bufs.s.rect.prog, "insts");
    bufs.s.rect.loc_inst_size = glGetUniformLocation(bufs.s.rect.prog, "inst_size");

    glGenBuffers(1, &bufs.s.rect.bo);
    glBindBuffer(GL_TEXTURE_BUFFER, bufs.s.rect.bo);
    glBufferData(GL_TEXTURE_BUFFER, CC_GL_MAX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

    glGenTextures(1, &bufs.s.rect.tbo);
    glBindTexture(GL_TEXTURE_BUFFER, bufs.s.rect.tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, bufs.s.rect.bo);

}

void cc_gl_rendererUpdate(s32 width, s32 height) {
    cc_mat4_orthographic(&proj2d, 0,width,height,0, -1,1);
}

void cc_gl_rendererDeinit(void) {
    free(batch.data);

    glDeleteProgram(bufs.s.rect.prog);
    glDeleteBuffers(1, &bufs.s.rect.tbo);
    glDeleteBuffers(1, &bufs.s.rect.bo);
    glDeleteVertexArrays(1, &bufs.s.rect.vao);
}


//
// FUNCS
//


void cc_gl_rendererAddInstance(GLinstanceData* data) {
    if (batch.data_size == batch.data_capac) {
        u32 newcapac = batch.data_capac == 0? 4 : batch.data_capac * 2;
        GLinstanceData* newdata = realloc(batch.data, newcapac * sizeof(GLinstanceData));
        if (!newdata) {
            printf("failed to grow batch!\n");
            exit(1);
        }
        batch.data = newdata;
        batch.data_capac = newcapac;
    }
    memcpy((char*)batch.data + batch.data_size * sizeof(GLinstanceData), data, sizeof(GLinstanceData));
    ++batch.data_size;
}

void cc_gl_rendererFlush(void) {
    if (batch.data_size == 0) return;

    switch (batch.type) {
        case CC_BATCH_NONE: break;
        case CC_2D_BATCH_RECT:
            glUseProgram(bufs.s.rect.prog);
            glBindVertexArray(bufs.s.rect.vao);

            glUniformMatrix4fv(bufs.s.rect.loc_proj, 1,0, proj2d);

            glBindBuffer(GL_TEXTURE_BUFFER, bufs.s.rect.bo);
            glBufferSubData(GL_TEXTURE_BUFFER, 0, batch.data_size * sizeof(GLinstanceData), batch.data);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_BUFFER, bufs.s.rect.tbo);
            glUniform1i(bufs.s.rect.loc_insts, 0);

            glUniform1i(bufs.s.rect.loc_inst_size, sizeof(GLinstanceData) >> 4);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, batch.data_size);

            //glBindTexture(GL_TEXTURE_BUFFER, 0);
            //glBindBuffer(GL_TEXTURE_BUFFER, 0);
            //glBindVertexArray(0);
            //glUseProgram(0);

            break;
    }

    batch.data_size = 0;
}

void cc_gl_rendererSetTint(f32 red, f32 green, f32 blue, f32 alpha) {
    r = red;
    g = green;
    b = blue;
    a = alpha;
}


// 2D
void cc_gl_rendererDrawRect(f32 x, f32 y, f32 w, f32 h) {
    if (batch.type != CC_2D_BATCH_RECT) cc_gl_rendererFlush();
    if (batch.data_size >= CC_GL_MAX_BATCH_SIZE) cc_gl_rendererFlush();

    batch.type = CC_2D_BATCH_RECT;

    cc_gl_rendererAddInstance(&(GLinstanceData){
        .x = x, .y = y, .w = w, .h = h,
        .r = r, .g = g, .b = b, .a = a
        });

    /*glUseProgram(bufs.s.rect.prog);
    glBindVertexArray(bufs.s.rect.vao);

    glUniformMatrix4fv(bufs.s.rect.loc_proj, 1,0, proj2d);
    glUniform1i(bufs.s.rect.loc_inst_size, sizeof(GLinstanceData) >> 4);

    glDrawArrays(GL_TRIANGLES, 0,6);

    glBindVertexArray(0);
    glUseProgram(0);*/
}

