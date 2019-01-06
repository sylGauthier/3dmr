#version 130

in vec3 direction;
out vec4 outputValue;

uniform sampler2D source;

#define PI 3.141592

vec2 cartesian_to_spherical(vec3 p) {
    float phi = atan(p.z, p.x);
    float theta = acos(p.y);
    return vec2(phi / (2.0 * PI) + 0.5, theta / PI);
}

void main() {
    outputValue = texture(source, cartesian_to_spherical(normalize(direction)));
}
