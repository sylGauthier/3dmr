#version 140

#include "camera.glsl"

in vec3 in_Vertex;
#ifdef HAVE_NORMAL
in vec3 in_Normal;
#endif
#ifdef HAVE_TEXCOORD
in vec2 in_TexCoord;
#endif
#if defined(HAVE_TANGENT) && defined(HAVE_NORMAL)
in vec3 in_Tangent;
in vec3 in_Bitangent;
#endif

#ifdef HAVE_NORMAL
out vec3 surfelPosition;
#ifdef HAVE_TANGENT
out mat3 tangentBasis;
#else
out vec3 surfelNormal;
#endif
#endif
#ifdef HAVE_TEXCOORD
out vec2 coordTexture;
#endif

uniform mat4 model;
#ifdef HAVE_NORMAL
uniform mat3 inverseNormal;
#endif

void main() {
    gl_Position = projection * view * model * vec4(in_Vertex, 1.0);
#ifdef HAVE_NORMAL
    surfelPosition = vec3(model * vec4(in_Vertex, 1.0));
#ifdef HAVE_TANGENT
    tangentBasis = inverseNormal * mat3(in_Tangent, in_Bitangent, in_Normal);
#else
    surfelNormal = normalize(inverseNormal * in_Normal);
#endif
#endif
#ifdef HAVE_TEXCOORD
    coordTexture = in_TexCoord;
#endif
}
