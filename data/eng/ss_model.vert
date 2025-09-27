#version 330 core

const vec2 verts[6] = vec2[6](
    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(1,1), vec2(0,1), vec2(0,0)
    );

uniform int inst_size;
uniform samplerBuffer insts;
uniform mat4 proj;
uniform mat4 cam_rot;
uniform mat4 cam_pos;
uniform float cam_z;
uniform float cam_tilt;

flat out vec4 col;
flat out vec2 samp_pos;
flat out vec2 samp_size;
out vec2 uv;

flat out float depth;

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

    mat4 rotation = mat4(
        texelFetch(insts,base+3),
        texelFetch(insts,base+4),
        texelFetch(insts,base+5),
        texelFetch(insts,base+6)
    );

    vec4 texel7 = texelFetch(insts,base+7);
    float z = texel7.x;
    float scale = texel7.y;
    float layer = texel7.z;

    vec4 vpos1 = cam_rot * vec4(
        pos * scale,
        0,1);

    vpos1 += cam_rot * rotation * vec4(
        (vert - .5f) * size * scale, 
        0,0);

    vec4 vpos2 = vec4(
        ((z * vec2(0,sin(1-abs(cam_tilt))) + layer * vec2(0,sin(1-abs(cam_tilt))*1.5)) * scale + cam_z) * vec2(0,1),
        -(z + layer + cam_z) * sign(cam_tilt) * scale * 1e5 + (vpos1.x+vpos1.y),
        0);

    vec4 glpos = proj * (cam_pos * (vpos1 * vec4(1,cam_tilt,1,1)) + vpos2);

    depth = glpos.z;

    gl_Position = glpos;
}
