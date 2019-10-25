#version 140

#ifdef HAVE_TEXCOORD
in vec2 coordTexture;
#endif

out vec4 out_Color;

#ifdef SOLID_TEXTURED
uniform sampler2D solidColor;
#else
uniform vec3 solidColor;
#endif

#include "alpha.glsl"

void main() {
#ifdef SOLID_TEXTURED
    vec3 color = texture(solidColor, coordTexture).rgb;
#else
    vec3 color = solidColor;
#endif
    out_Color = vec4(color, get_alpha());
}
