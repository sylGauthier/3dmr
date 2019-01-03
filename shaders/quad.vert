#version 130

out vec2 coordTexture;

void main() {
    if (gl_VertexID == 0) {
        gl_Position = vec4(-1,  1, 0, 1); coordTexture = vec2(0, 1);
    } else if (gl_VertexID == 1 || gl_VertexID == 4) {
        gl_Position = vec4(-1, -1, 0, 1); coordTexture = vec2(0, 0);
    } else if (gl_VertexID == 2 || gl_VertexID == 3) {
        gl_Position = vec4( 1,  1, 0, 1); coordTexture = vec2(1, 1);
    } else if (gl_VertexID == 5) {
        gl_Position = vec4( 1, -1, 0, 1); coordTexture = vec2(1, 0);
    }
}
