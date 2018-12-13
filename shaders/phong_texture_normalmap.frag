#version 130

#include "phong.glsl"

in vec3 surfelPosition;
in vec2 coordTexture;
in mat3 tangentBasis;
out vec4 out_Color;

uniform sampler2D tex;
uniform sampler2D normalMap;
uniform vec3 cameraPosition;

void main() {
    vec3 surfelNormal = normalize(2.0 * texture(normalMap, coordTexture).xyz - 1.0);
    surfelNormal = normalize(tangentBasis * surfelNormal);
    out_Color = vec4(texture(tex, coordTexture).xyz * phong(cameraPosition, surfelPosition, surfelNormal), 1.0);
}
