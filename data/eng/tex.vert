#version 330 core

const vec2 verts[6] = vec2[6](
    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(1,1), vec2(0,1), vec2(0,0)
    );

uniform int inst_size;
uniform samplerBuffer insts;
uniform mat4 proj;

flat out vec4 col;
flat out vec2 samp_pos;
flat out vec2 samp_size;
out vec2 uv;

void main() {
    vec2 vert = verts[gl_VertexID];
    int base = gl_InstanceID * inst_size;

    vec4 xywh = texelFetch(insts, base);
    vec4 rgba = texelFetch(insts, base+1);
    vec4 sxywh = texelFetch(insts, base+2);

    vec2 pos = xywh.xy;
    vec2 size = xywh.zw;
    col = rgba;
    samp_pos = sxywh.xy;
    samp_size = sxywh.zw;

    uv = vert;

    gl_Position = proj * vec4(vert * size + pos, 0,1);
}
