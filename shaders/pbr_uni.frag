#version 130

#include "pbr/pbr.glsl"
#include "hdr.glsl"

in vec3 surfelPosition;
in vec3 surfelNormal;
out vec4 out_Color;

uniform vec3 color;
uniform float metalness;
uniform float roughness;
uniform vec3 cameraPosition;

void main() {
    vec3 color = pbr(color, metalness, roughness, normalize(surfelNormal), surfelPosition, cameraPosition);
    color = reinhard_tonemapping(color, GAMMA, EXPOSURE, PURE_WHITE);
    out_Color = vec4(color, 1.0);
}
