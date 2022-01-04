#version 140

#if defined(SOLID_TEXTURED) || defined(ALPHA_TEXTURED)
#define HAVE_TEXCOORD
#endif

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
#ifdef FLOAT_TEXTURE
    vec3 color = vec3(texture(solidColor, coordTexture).r);
#else
    vec3 color = texture(solidColor, coordTexture).rgb;
#endif
#else
    vec3 color = solidColor;
#endif
    out_Color = vec4(color, get_alpha());
}
