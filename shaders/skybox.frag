#version 130

in vec3 direction;
out vec4 out_Color;

uniform samplerCube tex;

void main() {
    out_Color = texture(tex, direction);
}
