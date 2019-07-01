#version 140

in vec3 in_Vertex;
out vec3 direction;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * mat4(mat3(view)) * vec4(in_Vertex, 1.0);
    direction = in_Vertex;
}
