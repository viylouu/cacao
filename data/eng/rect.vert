#version 330 core

const vec2 verts[6] = vec2[6](
    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(1,1), vec2(0,1), vec2(0,0)
    );

uniform int inst_size;
uniform samplerBuffer insts;
uniform mat4 proj;

flat out vec4 col;

// why did i put "void maine()" bro...
void main() {
    vec2 vert = verts[gl_VertexID];
    int base = gl_InstanceID * inst_size;

    vec4 xywh = texelFetch(insts, base);
    vec4 rgba = texelFetch(insts, base+1);

    vec2 pos = xywh.xy;
    vec2 size = xywh.zw;
    col = rgba;

    mat4 transform = mat4(
        texelFetch(insts,base+3),
        texelFetch(insts,base+4),
        texelFetch(insts,base+5),
        texelFetch(insts,base+6)
    );

    gl_Position = proj * transform * vec4(vert * size + pos, 0,1);
}
