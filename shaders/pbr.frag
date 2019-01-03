#version 130

#include "pbr/pbr.glsl"
#include "hdr.glsl"

in vec3 surfelPosition;
in vec2 coordTexture;
in mat3 tangentBasis;
out vec4 out_Color;

uniform sampler2D tex;
uniform sampler2D normalMap;
uniform sampler2D metalnessTex;
uniform sampler2D roughnessTex;
uniform vec3 cameraPosition;

void main() {
    vec3 surfelNormal = normalize(2.0 * texture(normalMap, coordTexture).xyz - 1.0);
    surfelNormal = normalize(tangentBasis * surfelNormal);

    vec3 albedo = texture(tex, coordTexture).rgb;
    float metalness = texture(metalnessTex, coordTexture).r;
    float roughness = texture(roughnessTex, coordTexture).r;

    vec3 color = pbr(albedo, metalness, roughness, surfelNormal, surfelPosition, cameraPosition);
    color = reinhard_tonemapping(color, GAMMA, EXPOSURE, PURE_WHITE);
    out_Color = vec4(color, 1.0);
}
