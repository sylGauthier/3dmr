#version 130

#define NUM_SAMPLES (1024U)

#include "../pbr/util.glsl"
#include "util.glsl"

in vec2 coordTexture;
out vec4 outputValue;

// Schlick-GGX approximation of geometric attenuation function using Smith's method (IBL version).
float gaSchlickGGX_IBL(float cosLi, float cosLo, float roughness) {
	float r = roughness;
	float k = (r * r) / 2.0; // Epic suggests using this roughness remapping for IBL lighting.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

void main() {
	// Get integration parameters.
	float cosLo = coordTexture.x;
	float roughness = coordTexture.y;

	// Make sure viewing angle is non-zero to avoid divisions by zero (and subsequently NaNs).
	cosLo = max(cosLo, 0.001);

	// Derive tangent-space viewing vector from angle to normal (pointing towards +Z in this reference frame).
	vec3 Lo = vec3(sqrt(1.0 - cosLo * cosLo), 0.0, cosLo);

	// We will now pre-integrate Cook-Torrance BRDF for a solid white environment and save results into a 2D LUT.
	// DFG1 & DFG2 are terms of split-sum approximation of the reflectance integral.
	// For derivation see: "Moving Frostbite to Physically Based Rendering 3.0", SIGGRAPH 2014, section 4.9.2.
	float DFG1 = 0;
	float DFG2 = 0;

	for(uint i = 0U; i < NUM_SAMPLES; i++) {
		// Sample directly in tangent/shading space since we don't care about reference frame as long as it's consistent.
		vec3 Lh = sample_GGX(sample_hammersley(i, NUM_SAMPLES), roughness);
		// Compute incident direction (Li) by reflecting viewing direction (Lo) around half-vector (Lh).
		vec3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

		float cosLi   = Li.z;
		float cosLh   = Lh.z;
		float cosLoLh = max(dot(Lo, Lh), 0.0);

		if (cosLi > 0.0) {
			float G = gaSchlickGGX_IBL(cosLi, cosLo, roughness);
			float Gv = G * cosLoLh / (cosLh * cosLo);
			float Fc = pow(1.0 - cosLoLh, 5);

			DFG1 += (1 - Fc) * Gv;
			DFG2 += Fc * Gv;
		}
	}
    vec2 DFG = vec2(DFG1, DFG2) / vec2(NUM_SAMPLES);

	outputValue = vec4(DFG, 0, 1);
}
