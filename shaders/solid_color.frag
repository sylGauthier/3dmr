#version 130

out vec4 out_Color;

uniform vec3 solidColor;

void main() {
    out_Color = vec4(solidColor, 1.0);
}
