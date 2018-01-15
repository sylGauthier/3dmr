#version 130

in vec2 coordTexture;
out vec4 out_Color;

uniform sampler2D tex;
uniform vec3 textColor;

void main(void) {
     out_Color = vec4(textColor, texture2D(tex, coordTexture).r);
}