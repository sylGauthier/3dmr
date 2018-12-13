#version 130

in vec3 in_Vertex;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(in_Vertex, 1.0);
}
