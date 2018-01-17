#version 140

in vec3 in_Vertex;
in vec3 in_Normal;
out vec4 color;
out vec3 surfelPosition;
out vec3 surfelNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
//uniform mat3 NIT;
uniform vec3 solidColor;

void main()
{
    gl_Position = projection * view * model * vec4(in_Vertex, 1.0);
    color = vec4(solidColor, 1.0);

    mat3 NIT = transpose(inverse(mat3(model))); // HACK, shouldn't be computed there
    surfelPosition = vec3(model * vec4(in_Vertex, 1.0));
    surfelNormal = normalize(NIT * in_Normal);
}
