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
#ifdef HAVE_SKIN
in vec2 in_Index;
in vec2 in_Weight;
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
#ifdef HAVE_SKIN
layout(std140) uniform Bones {
    mat4 bones[NB_BONES];
};
#endif

void standard_vert_main() {
    vec4 skinnedVertex;
    vec4 skinnedNormal;

#ifdef HAVE_SKIN
    int index1, index2;
    index1 = int(in_Index.x);
    index2 = int(in_Index.y);
    skinnedVertex = (bones[index1] * vec4(in_Vertex, 1.0)) * in_Weight.x;
    skinnedVertex = skinnedVertex + (bones[index2] * vec4(in_Vertex, 1.0)) * in_Weight.y;
#else
    skinnedVertex = vec4(in_Vertex, 1.0);
#endif
#ifdef OVERLAY
    gl_Position = model * skinnedVertex;
#else
    gl_Position = projection * view * model * skinnedVertex;
#endif
#ifdef HAVE_NORMAL
#ifdef HAVE_SKIN
    skinnedNormal = (vec4(in_Normal, 0.0) * inverse(bones[index1])) * in_Weight.x;
    skinnedNormal = skinnedNormal + vec4(in_Normal, 0.0) * inverse(bones[index2]) * in_Weight.y;
#else
    skinnedNormal = vec4(in_Normal, 0.0);
#endif
    surfelPosition = vec3(model * vec4(skinnedVertex.xyz, 1.0));
#ifdef HAVE_TANGENT
    /*TODO: update with skinned tgt, bitgt and normal */
    tangentBasis = inverseNormal * mat3(in_Tangent, in_Bitangent, in_Normal);
#else
    surfelNormal = normalize(inverseNormal * skinnedNormal.xyz);
#endif
#endif
#ifdef HAVE_TEXCOORD
    coordTexture = in_TexCoord;
#endif
}
