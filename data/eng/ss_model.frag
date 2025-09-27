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
    
    /*float d = depth;
    //d /= 2147483647f;
    //d *= 156;
    //d += .5f;
    //d *= .5f;
    //d += .5f;
    d *= 8192000;*/

    oCol = /*vec4(d,d,d,1) */ samp;
}
