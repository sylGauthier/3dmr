#version 130

in vec3 in_Vertex;
out vec4 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 solidColor;

void main() {
    gl_Position = projection * view * model * vec4(in_Vertex, 1.0);
    color = vec4(solidColor, 1.0);
}
