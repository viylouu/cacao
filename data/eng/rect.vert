#version 330 core

const vec2 verts[6] = vec2[6](
    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(1,1), vec2(0,1), vec2(0,0)
    );

uniform vec2 pos;
uniform vec2 size;
uniform mat4 proj;

// why did i put "void maine()" bro...
void main() {
    vec2 vert = verts[gl_VertexID];

    gl_Position = proj * vec4(vert * size + pos, 0,1);
}
