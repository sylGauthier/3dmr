#version 130

in vec2 coordTexture;
out vec4 out_Color;

uniform sampler2D tex;

void main() {
    out_Color = texture(tex, coordTexture);
}
