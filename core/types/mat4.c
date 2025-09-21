#include "mat4.h"

#include <string.h>
#include <math.h>

void cc_mat4_make(
    mat4* mat,
    f32 m00, f32 m01, f32 m02, f32 m03,
    f32 m10, f32 m11, f32 m12, f32 m13,
    f32 m20, f32 m21, f32 m22, f32 m23,
    f32 m30, f32 m31, f32 m32, f32 m33
) {
    float tmp[16] = {
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33
    };
    memcpy(*mat, tmp, sizeof(tmp));
}

void cc_mat4_multiply(mat4* out, const mat4 a, const mat4 b) {
    f32 result[16];

    for (s32 row = 0; row < 4; row++) {
        for (s32 col = 0; col < 4; col++) {
            result[row*4 + col] =
                a[row*4 + 0] * b[0*4 + col] +
                a[row*4 + 1] * b[1*4 + col] +
                a[row*4 + 2] * b[2*4 + col] +
                a[row*4 + 3] * b[3*4 + col];
        }
    }

    memcpy(*out, result, sizeof(result));
}

void cc_mat4_orthographic(
    mat4* mat, 
    f32 left, f32 right,
    f32 bottom, f32 top,
    f32 near, f32 far
) {
    f32 rl = right - left;
    f32 tb = top - bottom;
    f32 fn = far - near;

    cc_mat4_make(mat, 
        2/rl, 0, 0, 0,
        0, 2/tb, 0, 0,
        0, 0,-2/fn, 0,
        -(right+left) / rl, -(top+bottom) / tb, -(far+near) / fn, 1
        );
}

void cc_mat4_identity(mat4* mat) {
     cc_mat4_make(mat, 
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        );
}

void cc_mat4_translate(
    mat4* mat,
    f32 x, f32 y, f32 z
) {
    cc_mat4_make(mat,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1
        );
}

void cc_mat4_scale(
    mat4* mat,
    f32 x, f32 y, f32 z
) {
    cc_mat4_make(mat,
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1
        );
}

void cc_mat4_rotateX(mat4* mat, f32 ang) {
    f32 c = cosf(ang);
    f32 s = sinf(ang);

    cc_mat4_make(mat,
        1, 0, 0, 0,
        0, c,-s, 0,
        0, s, c, 0,
        0, 0, 0, 1
        );
}

void cc_mat4_rotateY(mat4* mat, f32 ang) {
    f32 c = cosf(ang);
    f32 s = sinf(ang);

    cc_mat4_make(mat,
        c, 0, s, 0,
        0, 1, 0, 0,
       -s, 0, c, 0,
        0, 0, 0, 1
        );
}

void cc_mat4_rotateZ(mat4* mat, f32 ang) {
    f32 c = cosf(ang);
    f32 s = sinf(ang);

    cc_mat4_make(mat,
        c,-s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        );
}
