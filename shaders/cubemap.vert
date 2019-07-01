#version 140

out vec3 direction;

uniform int face;

void main() {
    if (gl_VertexID == 0) {
        gl_Position = vec4(-1,  1, 0, 1);
    } else if (gl_VertexID == 1 || gl_VertexID == 4) {
        gl_Position = vec4(-1, -1, 0, 1);
    } else if (gl_VertexID == 2 || gl_VertexID == 3) {
        gl_Position = vec4( 1,  1, 0, 1);
    } else if (gl_VertexID == 5) {
        gl_Position = vec4( 1, -1, 0, 1);
    }

    if (face == 0) {
        direction = vec3(1, -gl_Position.y, -gl_Position.x);
    } else if (face == 1) {
        direction = vec3(-1, -gl_Position.y, gl_Position.x);
    } else if (face == 2) {
        direction = vec3(gl_Position.x, 1, gl_Position.y);
    } else if (face == 3) {
        direction = vec3(gl_Position.x, -1, -gl_Position.y);
    } else if (face == 4) {
        direction = vec3(gl_Position.x, -gl_Position.y, 1);
    } else if (face == 5) {
        direction = vec3(-gl_Position.x, -gl_Position.y, -1);
    }
}
