#include "renderer.h"

#include <stdio.h>
#include <GL/gl.h>

#define FUNC(name, ret_type, ...) \
    typedef ret_type (APIENTRYP* name##_t)(__VA_ARGS__); \
    name##_t name = NULL

#ifdef _WIN32
    #include <windows.h>

    #define LOAD(name) \
                do {
                    name = (name##_t)wglGetProcAddress(#name); \
                    if (!name) { printf("failed to load %s!\n", #name); return }
                } while (0)
#else
    #include <EGL/egl.h>
    #include <GL/glx.h>

    #define LOAD(name) \
                do { \
                    if (cc_renderer_use_wayland) name = (name##_t)eglGetProcAddress(#name); \
                    else name = (name##_t)glXGetProcAddress((const GLubyte*)name); \
                    if (!name) { printf("failed to load %s!\n", #name); return; } \
                } while(0)
#endif

FUNC(glCreateShader, GLuint, GLenum type);
FUNC(glShaderSource, void, GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
FUNC(glCompileShader, void, GLuint shader);
FUNC(glGetShaderiv, void, GLuint shader, GLenum pname, GLint* params);
FUNC(glGetShaderInfoLog, void, GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infolog);
FUNC(glDeleteShader, void, GLuint shader);

FUNC(glCreateProgram, GLuint, void);
FUNC(glAttachShader, void, GLuint program, GLuint shader);
FUNC(glLinkProgram, void, GLuint program);
FUNC(glGetProgramiv, void, GLuint program, GLenum pname, GLint* params);
FUNC(glGetProgramInfoLog, void, GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
FUNC(glDeleteProgram, void, GLuint program);

FUNC(glGenVertexArrays, void, GLsizei n, GLuint* arrays);
FUNC(glGetUniformLocation, GLint, GLuint program, const GLchar* name);
FUNC(glGenBuffers, void, GLsizei n, GLuint* buffers);
FUNC(glBindBuffer, void, GLenum target, GLuint buffer);
FUNC(glBufferData, void, GLenum target, GLsizeiptr size, const void* data, GLenum usage);
FUNC(glTexBuffer, void, GLenum target, GLenum internalformat, GLuint buffer);

FUNC(glDeleteVertexArrays, void, GLsizei n, const GLuint* arrays);
FUNC(glDeleteBuffers, void, GLsizei n, const GLuint* buffers);

FUNC(glUseProgram, void, GLuint program);
FUNC(glBindVertexArray, void, GLuint array);
FUNC(glUniformMatrix4fv, void, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
FUNC(glBufferSubData, void, GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
FUNC(glUniform1i, void, GLint location, GLint v0);
FUNC(glDrawArraysInstanced, void, GLenum mode, GLint first, GLsizei count, GLsizei instancecount);

void cc_gl_load(void) {
    LOAD(glCreateShader);
    LOAD(glShaderSource);
    LOAD(glCompileShader);
    LOAD(glGetShaderiv);
    LOAD(glGetShaderInfoLog);
    LOAD(glDeleteShader);

    LOAD(glCreateProgram);
    LOAD(glAttachShader);
    LOAD(glLinkProgram);
    LOAD(glGetProgramiv);
    LOAD(glGetProgramInfoLog);
    LOAD(glDeleteProgram);

    LOAD(glGenVertexArrays);
    LOAD(glGetUniformLocation);
    LOAD(glGenBuffers);
    LOAD(glBindBuffer);
    LOAD(glBufferData);
    LOAD(glTexBuffer);

    LOAD(glDeleteVertexArrays);
    LOAD(glDeleteBuffers);

    LOAD(glUseProgram);
    LOAD(glBindVertexArray);
    LOAD(glUniformMatrix4fv);
    LOAD(glBufferSubData);
    LOAD(glUniform1i);
    LOAD(glDrawArraysInstanced);
}
