#version 140

#include "pbr/pbr.glsl"
#include "hdr.glsl"

#ifndef HAVE_NORMAL
#error "PBR shader needs normals"
#endif

in vec3 surfelPosition;
#ifdef HAVE_TEXCOORD
in vec2 coordTexture;
#endif
#ifdef HAVE_TANGENT
in mat3 tangentBasis;
#else
in vec3 surfelNormal;
#endif
out vec4 out_Color;

#ifdef HAVE_TEXCOORD
uniform sampler2D tex;
uniform sampler2D metalnessTex;
uniform sampler2D roughnessTex;
#ifdef HAVE_TANGENT
uniform sampler2D normalMap;
#endif
#else
uniform vec3 albedo;
uniform float metalness;
uniform float roughness;
#endif
uniform vec3 cameraPosition;

void main() {
#ifdef HAVE_TANGENT
    vec3 surfelNormal = normalize(2.0 * texture(normalMap, coordTexture).xyz - 1.0);
    surfelNormal = tangentBasis * surfelNormal;
#endif

#ifdef HAVE_TEXCOORD
    vec3 albedo = texture(tex, coordTexture).rgb;
    float metalness = texture(metalnessTex, coordTexture).r;
    float roughness = texture(roughnessTex, coordTexture).r;
#endif

    vec3 color = pbr(albedo, metalness, roughness, normalize(surfelNormal), surfelPosition, cameraPosition);
    color = reinhard_tonemapping(color, GAMMA, EXPOSURE, PURE_WHITE);
    out_Color = vec4(color, 1.0);
}
