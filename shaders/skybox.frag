#version 130

#include "hdr.glsl"

in vec3 direction;
out vec4 out_Color;

uniform samplerCube tex;

void main() {
    out_Color = vec4(reinhard_tonemapping(texture(tex, direction).rgb, GAMMA, EXPOSURE, PURE_WHITE), 1.0);
}
