#include "camera.glsl"

in vec3 in_Vertex;

#ifdef HAVE_NORMAL
in vec3 in_Normal;
#ifdef HAVE_TANGENT
in vec3 in_Tangent;
in vec3 in_Bitangent;
#endif
#endif

#ifdef HAVE_TEXCOORD
in vec2 in_TexCoord;
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

#ifdef HAVE_SKIN
    mat4 skinTransform = mat4(0);
    mat3 skinInverseNormal;
    uint i;
    for (i = 0U; i < 2U; i++) {
        skinTransform += bones[int(in_Index[i])] * in_Weight[i];
    }
    skinInverseNormal = transpose(inverse(mat3(skinTransform)));
    skinnedVertex = skinTransform * vec4(in_Vertex, 1.0);
#else
    skinnedVertex = vec4(in_Vertex, 1.0);
#endif

#ifdef OVERLAY
    gl_Position = model * skinnedVertex;
#else
    gl_Position = projection * view * model * skinnedVertex;
#endif

#ifdef HAVE_NORMAL
    surfelPosition = vec3(model * vec4(skinnedVertex.xyz, 1.0));
#ifdef HAVE_TANGENT
#ifdef HAVE_SKIN
    tangentBasis = inverseNormal * skinInverseNormal * mat3(in_Tangent, in_Bitangent, in_Normal);
#else
    tangentBasis = inverseNormal * mat3(in_Tangent, in_Bitangent, in_Normal);
#endif
#else
#ifdef HAVE_SKIN
    surfelNormal = normalize(inverseNormal * skinInverseNormal * in_Normal);
#else
    surfelNormal = normalize(inverseNormal * in_Normal);
#endif
#endif
#endif

#ifdef HAVE_TEXCOORD
    coordTexture = in_TexCoord;
#endif
}
