#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stbimage.h>

#include <GL/gl.h>
#include <stdio.h>
#include "renderer_gl_loader.h"
#include <core/types/mat4.h>
#include <string.h>

// table of contents:
//  VARS
//  TEXTURES
//  SHADERS
//  SPRITESTACKS
//  MAIN
//  FUNCS
//      2D
//      SPRITESTACK
//


//
// VARS
//

struct {
    // naming convention:
    //  its basically just whats easy for me to type
    //  1 - a
    //  2 - s
    //  3 - d
    //  4 - f
    //  etc.
    //  2.5 - sg (isometric)
    //  spritestack - ss

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
        
        struct {
            u32 vao;
            u32 prog;
            u32 bo;
            u32 tbo;
            s32 loc_inst_size;
            s32 loc_insts;
            s32 loc_proj;
            s32 loc_tex;
        } tex;
    } s;
    struct {
        struct {
            u32 vao;
            u32 prog;
            u32 bo;
            u32 tbo;
            s32 loc_inst_size;
            s32 loc_insts;
            s32 loc_proj;
            s32 loc_tex;
            s32 loc_cam_pos;
            s32 loc_cam_rot;
        } model;
    } ss;
} bufs;

b8 cc_renderer_use_wayland;

mat4 proj2d;

mat4 transform;

f32 r,g,b,a;

const s32 CC_GL_MAX_BATCH_SIZE = 65536;
s32 CC_GL_MAX_BUFFER_SIZE;

#pragma pack(4)
typedef struct {
    f32 x, y, w, h;
    f32 r, g, b, a;
    f32 sx,sy,sw,sh;
    mat4 transform; // todo: figure out how thisll work (omfg so silly :DDDDD)
    f32 z;
    f32 layer;
    f32 _pad0, _pad1;
} GLinstanceData;
#pragma pack()

typedef enum {
    CC_BATCH_NONE,
    CC_2D_BATCH_RECT,
    CC_2D_BATCH_TEXTURE
} CCbatchType;

struct {
    CCbatchType type;
    GLinstanceData* data;
    u32 data_size;
    u32 data_capac;
    CCtexture* tex;
} batch;


//
// TEXTURES
//


void cc_gl_unloadTexture(CCtexture* tex) {
    glDeleteTextures(1, &((GLtexture*)tex->platform_specific)->id);
    free(tex);
}

CCtexture* cc_gl_loadTextureFromData(u8* data, size_t size) {
    s32 w, h, c;
    u8* texdata = stbi_load_from_memory(data, size, &w,&h,&c, 4);
    if (!texdata) { printf("failed to load texture!\n"); exit(1); }

    u32 id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);

    glBindTexture(GL_TEXTURE_2D, 0);

    CCtexture* tex = malloc(sizeof(CCtexture));
    tex->platform_specific = malloc(sizeof(GLtexture));
    ((GLtexture*)tex->platform_specific)->id = id;
    tex->width = w;
    tex->height = h;

    return tex;
}

char* cc_gl_loadTextureData(const char* path, size_t* out_size) {
    FILE* file = fopen(path, "rb");
    if (!file) { printf("failed to load texture at \"%s\"!\n", path); exit(1); }

    fseek(file, 0, SEEK_END);
    long size = ftell(file); // why is it long and not long long so i can use s64? idfk
    rewind(file);

    char* buffer = malloc(size);
    if (!buffer) {
        fclose(file);
        printf("failed to allocate size for the texture at \"%s\"!\n", path);
        exit(1);
    }

    fread(buffer, 1, size, file);
    fclose(file);

    *out_size = size;

    return buffer;
}

CCtexture* cc_gl_loadTexture(const char* path) {
    size_t size;
    char* buf = cc_gl_loadTextureData(path, &size);
    if (!buf) {
        printf("failed to read file at \"%s\"!\n", path);
        exit(1);
    }

    CCtexture* tex = cc_gl_loadTextureFromData((u8*)buf, size);
    free(buf);

    if (!tex) {
        printf("failed to load texture at \"%s\"!\n", path);
        exit(1);
    }

    return tex;
}


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

char* cc_gl_loadShaderSource(const char* path) {
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
    char* buffer = cc_gl_loadShaderSource(path);
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
    char* vbuf = cc_gl_loadShaderSource(vert);
    char* fbuf = cc_gl_loadShaderSource(frag);

    const char* vsrc = vbuf;
    const char* fsrc = fbuf;

    u32 program = cc_gl_loadProgramFromSource(&vsrc, &fsrc);

    free(vbuf);
    free(fbuf);

    return program;
}


//
// SPRITESTACKS
//


CCspriteStack* cc_gl_loadSpriteStack(const char* path, f32 layerheight) {
    CCspriteStack* stack = malloc(sizeof(CCspriteStack));
    stack->texture = cc_gl_loadTexture(path);
    stack->layer_height = layerheight;
    stack->layers = stack->texture->height / layerheight;

    // this is incase silly vulkan needs anything (i doubt it, but... idk)
    stack->platform_specific = malloc(sizeof(u8));

    return stack;
}

void cc_gl_unloadSpriteStack(CCspriteStack* stack) {
    cc_gl_unloadTexture(stack->texture);
    free(stack->platform_specific);
    free(stack);
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

    //
    // 2D
    //

    // RECT
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

    // TEX
    bufs.s.tex.prog = cc_gl_loadProgram("data/eng/tex.vert", "data/eng/tex.frag");
    glGenVertexArrays(1, &bufs.s.tex.vao);

    bufs.s.tex.loc_proj      = glGetUniformLocation(bufs.s.tex.prog, "proj");
    bufs.s.tex.loc_insts     = glGetUniformLocation(bufs.s.tex.prog, "insts");
    bufs.s.tex.loc_inst_size = glGetUniformLocation(bufs.s.tex.prog, "inst_size");
    bufs.s.tex.loc_tex       = glGetUniformLocation(bufs.s.tex.prog, "tex");

    glGenBuffers(1, &bufs.s.tex.bo);
    glBindBuffer(GL_TEXTURE_BUFFER, bufs.s.tex.bo);
    glBufferData(GL_TEXTURE_BUFFER, CC_GL_MAX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

    glGenTextures(1, &bufs.s.tex.tbo);
    glBindTexture(GL_TEXTURE_BUFFER, bufs.s.tex.tbo);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, bufs.s.tex.bo);

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

    glDeleteProgram(bufs.s.tex.prog);
    glDeleteBuffers(1, &bufs.s.tex.tbo);
    glDeleteBuffers(1, &bufs.s.tex.bo);
    glDeleteVertexArrays(1, &bufs.s.tex.vao);
}


//
// FUNCS
//


void cc_gl_rendererAddInstance(GLinstanceData* data) {
    // idgaf its for the end user experience!!! (also me)
    memcpy(&data->r, (f32[4]){r,g,b,a}, sizeof(f32)*4);
    memcpy(&data->transform, transform, sizeof(mat4));

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

            glUniform1i(bufs.s.rect.loc_inst_size, sizeof(GLinstanceData) / 16);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, batch.data_size);

            break;
        case CC_2D_BATCH_TEXTURE:
            glUseProgram(bufs.s.tex.prog);
            glBindVertexArray(bufs.s.tex.vao);

            glUniformMatrix4fv(bufs.s.tex.loc_proj, 1,0, proj2d);

            glBindBuffer(GL_TEXTURE_BUFFER, bufs.s.tex.bo);
            glBufferSubData(GL_TEXTURE_BUFFER, 0, batch.data_size * sizeof(GLinstanceData), batch.data);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_BUFFER, bufs.s.tex.tbo);
            glUniform1i(bufs.s.tex.loc_insts, 0);

            glUniform1i(bufs.s.tex.loc_inst_size, sizeof(GLinstanceData) / 16);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, ((GLtexture*)batch.tex->platform_specific)->id);
            glUniform1i(bufs.s.tex.loc_tex, 1);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, batch.data_size);

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

void cc_gl_rendererClear(f32 r, f32 g, f32 b, f32 a) {
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void cc_gl_rendererResetTransform(void) {
    cc_mat4_identity(&transform);
}

void cc_gl_rendererGetTransform(mat4* out) {
    memcpy(out, &transform, sizeof(mat4));
}

void cc_gl_rendererSetTransform(mat4* matrix) {
    memcpy(transform, matrix, sizeof(mat4));
}

void cc_gl_rendererTranslate(float x, float y, float z) {
    mat4 a;
    cc_mat4_translate(&a, x,y,z);
    cc_mat4_multiply(&transform, transform, a);
}

void cc_gl_rendererScale(float x, float y, float z) {
    mat4 a;
    cc_mat4_scale(&a, x,y,z);
    cc_mat4_multiply(&transform, transform, a);
}

void cc_gl_rendererRotate(float x, float y, float z) {
    mat4 rx, ry, rz;
    cc_mat4_rotateX(&rx, x);
    cc_mat4_rotateY(&ry, y);
    cc_mat4_rotateZ(&rz, z);
    cc_mat4_multiply(&rx, rx, ry);
    cc_mat4_multiply(&rz, rx, rz);
    cc_mat4_multiply(&transform, transform, rz);
}


// 2D
void cc_gl_rendererDrawRect(f32 x, f32 y, f32 w, f32 h) {
    if (batch.type != CC_2D_BATCH_RECT) cc_gl_rendererFlush();
    if (batch.data_size >= CC_GL_MAX_BATCH_SIZE) cc_gl_rendererFlush();

    batch.type = CC_2D_BATCH_RECT;

    cc_gl_rendererAddInstance(&(GLinstanceData){
        .x = x, .y = y, .w = w, .h = h,
        });
}

void cc_gl_rendererDrawTexture(CCtexture* tex, f32 x, f32 y, f32 w, f32 h, f32 sx, f32 sy, f32 sw, f32 sh) {
    if (batch.type != CC_2D_BATCH_TEXTURE) cc_gl_rendererFlush();
    if (batch.data_size >= CC_GL_MAX_BATCH_SIZE) cc_gl_rendererFlush();
    if (!tex) { printf("tf do you expect? you need to initialize a texture!\n"); exit(1); }

    batch.type = CC_2D_BATCH_TEXTURE;
    batch.tex = tex;

    cc_gl_rendererAddInstance(&(GLinstanceData){
        .x = x, .y = y, .w = w, .h = h,
        .sx = sx / tex->width, .sy = sy / tex->height, .sw = sw / tex->width, .sh = sh / tex->height
        });
}

// SPRITESTACK
void cc_gl_rendererDrawSpriteStack(CCspriteStack* stack, f32 x, f32 y, f32 z, f32 scale, f32 rotation) {
    // just getting this basic ahh in here temporarily
    // TODO: optimize ts, use depth buffers, tbos, bos, vaos, etcos.
    
    cc_gl_rendererScale(scale,scale,1);

    mat4 prev;
    cc_rendererGetTransform(&prev);

    for (f32 i = stack->layers-1; i >= 0; i -= 1.f/scale) {
        cc_gl_rendererTranslate(-stack->texture->width * scale * .5f, -stack->layer_height * scale * .5f, 0);
        cc_gl_rendererRotate(0,0,rotation);

        cc_gl_rendererTranslate(x,y,0);
        cc_gl_rendererTranslate(0,(i+z)*scale,0);

        cc_gl_rendererDrawTexture(stack->texture, 0,0,stack->texture->width, stack->layer_height, 0, (s32)i*stack->layer_height, stack->texture->width, stack->layer_height);
        cc_gl_rendererSetTransform(&prev);
    }
}
