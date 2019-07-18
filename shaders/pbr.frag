#version 140

#include "camera.glsl"
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

#ifdef ALBEDO_TEXTURED
uniform sampler2D albedo;
#else
uniform vec3 albedo;
#endif
#ifdef METALNESS_TEXTURED
uniform sampler2D metalness;
#else
uniform float metalness;
#endif
#ifdef ROUGHNESS_TEXTURED
uniform sampler2D roughness;
#else
uniform float roughness;
#endif
#ifdef HAVE_TANGENT
uniform sampler2D normalMap;
#endif

void main() {
#ifdef HAVE_TANGENT
    vec3 surfelNormal = normalize(2.0 * texture(normalMap, coordTexture).xyz - 1.0);
    surfelNormal = tangentBasis * surfelNormal;
#endif

#ifdef ALBEDO_TEXTURED
    vec3 a = texture(albedo, coordTexture).rgb;
#else
    vec3 a = albedo;
#endif
#ifdef METALNESS_TEXTURED
    float m = texture(metalness, coordTexture).r;
#else
    float m = metalness;
#endif
#ifdef ROUGHNESS_TEXTURED
    float r = texture(roughness, coordTexture).r;
#else
    float r = roughness;
#endif

    vec3 color = pbr(a, m, r, normalize(surfelNormal), surfelPosition, cameraPosition);
    color = reinhard_tonemapping(color, GAMMA, EXPOSURE, PURE_WHITE);
    out_Color = vec4(color, 1.0);
}
