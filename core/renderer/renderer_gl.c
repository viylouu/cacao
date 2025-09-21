#include "renderer.h"

#include <GL/gl.h>
#include <stdio.h>
#include "renderer_gl_loader.h"
#include <core/types/mat4.h>

// table of contents:
//  TYPES
//  SHADERS
//  MAIN
//  FUNCS
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
            s32 loc_pos;
            s32 loc_size;
            s32 loc_col;
            s32 loc_proj;
        } rect;
    } s;
} bufsgl;

b8 cc_renderer_use_wayland;

mat4 proj2d;

f32 r,g,b,a;


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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    bufsgl.s.rect.prog = cc_gl_loadProgram("data/eng/rect.vert", "data/eng/rect.frag");
    glGenVertexArrays(1, &bufsgl.s.rect.vao);

    bufsgl.s.rect.loc_pos  = glGetUniformLocation(bufsgl.s.rect.prog, "pos");
    bufsgl.s.rect.loc_size = glGetUniformLocation(bufsgl.s.rect.prog, "size");
    bufsgl.s.rect.loc_col  = glGetUniformLocation(bufsgl.s.rect.prog, "col");
    bufsgl.s.rect.loc_proj = glGetUniformLocation(bufsgl.s.rect.prog, "proj");

}

void cc_gl_rendererUpdate(s32 width, s32 height) {
    cc_mat4_orthographic(&proj2d, 0,width,height,0, -1,1);
}

void cc_gl_rendererDeinit(void) {
    glDeleteVertexArrays(1, &bufsgl.s.rect.vao);
}


//
// FUNCS
//


void cc_gl_rendererSetTint(f32 red, f32 green, f32 blue, f32 alpha) {
    r = red;
    g = green;
    b = blue;
    a = alpha;
}

void cc_gl_rendererDrawRect(f32 x, f32 y, f32 w, f32 h) {
    glUseProgram(bufsgl.s.rect.prog);
    glBindVertexArray(bufsgl.s.rect.vao);

    glUniformMatrix4fv(bufsgl.s.rect.loc_proj, 1,0, proj2d);
    glUniform2f(bufsgl.s.rect.loc_pos, x,y);
    glUniform2f(bufsgl.s.rect.loc_size, w,h);
    glUniform4f(bufsgl.s.rect.loc_col, r,g,b,a);

    glDrawArrays(GL_TRIANGLES, 0,6);

    glBindVertexArray(0);
    glUseProgram(0);
}
