#version 330 core

uniform sampler2D tex;

in vec2 uvs;

out vec4 fCol;

void main() {
    fCol = texture(tex, uvs);
}

