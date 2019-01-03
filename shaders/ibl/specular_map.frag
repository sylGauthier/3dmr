#version 130

#define NUM_SAMPLES (1024U)

#include "../pbr/util.glsl"
#include "util.glsl"

in vec3 direction;
out vec4 outputValue;

uniform samplerCube envmap;
uniform float roughness;

void main() {
    vec3 N = normalize(direction);
	vec3 Lo = N;
    mat3 tangentBasis = compute_tangent_basis(N);
    vec3 color = vec3(0);
	float weight = 0;

	// Solid angle associated with a single cubemap texel at zero mipmap level.
	// This will come in handy for importance sampling below.
	vec2 inputSize = vec2(textureSize(envmap, 0));
	float wt = 4.0 * PI / (6 * inputSize.x * inputSize.y);

    for (uint i = 0U; i < NUM_SAMPLES; i++) {
        vec3 Lh = tangentBasis * sample_GGX(sample_hammersley(i, NUM_SAMPLES), roughness);
		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
		vec3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi = dot(N, Li);
		if (cosLi > 0.0) {
			// Use Mipmap Filtered Importance Sampling to improve convergence.
			// See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

			float cosLh = max(dot(N, Lh), 0.0);

			// GGX normal distribution function (D term) probability density function.
			// Scaling by 1/4 is due to change of density in terms of Lh to Li (and since N=V, rest of the scaling factor cancels out).
			float pdf = ndfGGX(cosLh, roughness) * 0.25;

			// Solid angle associated with this sample.
			float ws = 1.0 / (NUM_SAMPLES * pdf);

			// Mip level to sample from.
			float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);

			color += textureLod(envmap, Li, mipLevel).rgb * cosLi;
			weight += cosLi;
		}
    }
	color /= weight;

    outputValue = vec4(color, 1.0);
}
