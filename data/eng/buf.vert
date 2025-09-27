#version 330 core

const vec2 verts[6] = vec2[6](
        vec2(-1,-1), vec2(-1,1),
        vec2(1,1), vec2(1,1),
        vec2(1,-1), vec2(-1,-1)
    );

out vec2 uvs;

void main() {
    vec2 val = verts[gl_VertexID];
    gl_Position = vec4(val, 0,1);
    uvs = val *.5+.5;
}
