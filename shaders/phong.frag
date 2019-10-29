#version 140

#include "camera.glsl"
#include "phong.glsl"

#if defined(AMBIENT_TEXTURED) || defined(DIFFUSE_TEXTURED) || defined(SPECULAR_TEXTURED) || defined(SHININESS_TEXTURED) || defined(NORMALMAP) || defined(ALPHA_TEXTURED)
#define HAVE_TEXCOORD
#endif

in vec3 surfelPosition;
#ifdef NORMALMAP
in mat3 tangentBasis;
#else
in vec3 surfelNormal;
#endif
#ifdef HAVE_TEXCOORD
in vec2 coordTexture;
#endif

out vec4 out_Color;

#ifdef AMBIENT_TEXTURED
uniform sampler2D ambient;
#else
uniform vec3 ambient;
#endif
#ifdef DIFFUSE_TEXTURED
uniform sampler2D diffuse;
#else
uniform vec3 diffuse;
#endif
#ifdef SPECULAR_TEXTURED
uniform sampler2D specular;
#else
uniform vec3 specular;
#endif
#ifdef SHININESS_TEXTURED
uniform sampler2D shininess;
#else
uniform float shininess;
#endif
#ifdef NORMALMAP
uniform sampler2D normalMap;
#endif

#include "alpha.glsl"

void main() {
#ifdef NORMALMAP
    vec3 surfelNormal = normalize(2.0 * texture(normalMap, coordTexture).xyz - 1.0);
    surfelNormal = tangentBasis * surfelNormal;
#endif
#ifdef AMBIENT_TEXTURED
    vec3 a = texture(ambient, coordTexture).xyz;
#else
    vec3 a = ambient;
#endif
#ifdef DIFFUSE_TEXTURED
    vec3 d = texture(diffuse, coordTexture).xyz;
#else
    vec3 d = diffuse;
#endif
#ifdef SPECULAR_TEXTURED
    vec3 s = texture(specular, coordTexture).xyz;
#else
    vec3 s = specular;
#endif
#ifdef SHININESS_TEXTURED
    float sh = texture(shininess, coordTexture).r;
#else
    float sh = shininess;
#endif
    out_Color = vec4(phong(a, d, s, sh, cameraPosition, surfelPosition, normalize(surfelNormal)), get_alpha());
}
