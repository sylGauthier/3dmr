#version 140

#ifdef HAVE_TEXCOORD
in vec2 coordTexture;
#endif

out vec4 out_Color;

#ifdef HAVE_TEXCOORD
uniform sampler2D solidColor;
#else
uniform vec3 solidColor;
#endif

void main() {
#ifdef HAVE_TEXCOORD
    vec3 color = texture(solidColor, coordTexture).rgb;
#else
    vec3 color = solidColor;
#endif
    out_Color = vec4(color, 1.0);
}
