#version 330 core

uniform sampler2D tex;

flat in vec4 col;
flat in vec2 samp_pos;
flat in vec2 samp_size;
flat in float depth;
in vec2 uv;

out vec4 oCol;

void main() {
    vec4 samp = texture(tex, uv * samp_size + samp_pos) * col;
    if (samp.a == 0) discard;

    oCol = samp;
}
