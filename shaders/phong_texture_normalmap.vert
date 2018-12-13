#version 130

in vec3 in_Vertex;
in vec3 in_Normal;
in vec2 in_TexCoord;
in vec3 in_Tangent;
in vec3 in_Bitangent;
out vec2 coordTexture;
out vec3 surfelPosition;
out mat3 tangentBasis;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 inverseNormal;

void main() {
    gl_Position = projection * view * model * vec4(in_Vertex, 1.0);
    coordTexture = in_TexCoord;

    surfelPosition = vec3(model * vec4(in_Vertex, 1.0));
    tangentBasis = inverseNormal * mat3(in_Tangent, in_Bitangent, in_Normal);
}
