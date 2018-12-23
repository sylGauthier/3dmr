#version 130

in vec3 in_Vertex;
in vec3 in_Normal;
in vec2 in_TexCoord;
out vec2 coordTexture;
out vec3 surfelPosition;
out vec3 surfelNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 inverseNormal;

void main() {
    gl_Position = projection * view * model * vec4(in_Vertex, 1.0);
    coordTexture = in_TexCoord;

    surfelPosition = vec3(model * vec4(in_Vertex, 1.0));
    surfelNormal = normalize(inverseNormal * in_Normal);
}
