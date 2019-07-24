#version 140

#include "camera.glsl"

in vec3 in_Vertex;
out vec3 direction;

void main() {
    gl_Position = projection * mat4(mat3(view)) * vec4(in_Vertex, 1.0);
    gl_Position = vec4(gl_Position.xy / gl_Position.z, 0.9999999, 1);
    direction = in_Vertex;
}
