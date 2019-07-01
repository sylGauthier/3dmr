#version 140

#include "phong.glsl"

#ifndef HAVE_NORMAL
#error "Phong shader needs normals"
#endif

in vec3 surfelPosition;
#ifdef HAVE_TANGENT
in mat3 tangentBasis;
#else
in vec3 surfelNormal;
#endif
#ifdef HAVE_TEXCOORD
in vec2 coordTexture;
#endif

out vec4 out_Color;

#ifdef HAVE_TEXCOORD
uniform sampler2D tex;
#ifdef HAVE_TANGENT
uniform sampler2D normalMap;
#endif
#else
uniform vec3 color;
#endif
uniform vec3 cameraPosition;

void main() {
#ifdef HAVE_TANGENT
    vec3 surfelNormal = normalize(2.0 * texture(normalMap, coordTexture).xyz - 1.0);
    surfelNormal = tangentBasis * surfelNormal;
#endif
#ifdef HAVE_TEXCOORD
    vec3 color = texture(tex, coordTexture).xyz;
#endif
    out_Color = vec4(color * phong(cameraPosition, surfelPosition, normalize(surfelNormal)), 1.0);
}
