#version 130

#define NUM_SAMPLES (64U * 1024U)

#include "util.glsl"

in vec3 direction;
out vec4 outputValue;

uniform samplerCube envmap;

void main() {
    vec3 N = normalize(direction);
    mat3 tangentBasis = compute_tangent_basis(N);
    vec3 irradiance = vec3(0);

    for (uint i = 0U; i < NUM_SAMPLES; i++) {
        vec3 Li = tangentBasis * sample_hemisphere_uniform(sample_hammersley(i, NUM_SAMPLES));
        float cosTheta = max(0.0, dot(Li, N));
        irradiance += 2.0 * texture(envmap, Li).rgb * cosTheta;
    }
    irradiance /= vec3(NUM_SAMPLES);

    outputValue = vec4(irradiance, 1.0);
}
