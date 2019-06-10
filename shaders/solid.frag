#version 130

#ifdef HAVE_TEXCOORD
in vec2 coordTexture;
#endif

out vec4 out_Color;

#ifdef HAVE_TEXCOORD
uniform sampler2D tex;
#else
uniform vec3 solidColor;
#endif

void main() {
#ifdef HAVE_TEXCOORD
    vec3 solidColor = texture(tex, coordTexture).rgb;
#endif
    out_Color = vec4(solidColor, 1.0);
}
