#version 130

#include "phong.glsl"

in vec3 surfelPosition;
in vec3 surfelNormal;
in vec4 color;
out vec4 out_Color;

uniform vec3 cameraPosition;

void main() {
    out_Color = color * vec4(phong_light_factor(cameraPosition, surfelPosition, surfelNormal), 1.0);
}
