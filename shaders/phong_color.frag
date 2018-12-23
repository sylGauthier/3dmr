#version 130

#include "phong.glsl"

in vec3 surfelPosition;
in vec3 surfelNormal;
out vec4 out_Color;

uniform vec3 color;
uniform vec3 cameraPosition;

void main() {
    out_Color = vec4(color * phong(cameraPosition, surfelPosition, surfelNormal), 1.0);
}
