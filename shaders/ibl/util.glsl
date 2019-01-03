#define PI 3.141592

// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

float radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 sample_hammersley(uint i, uint n) {
    return vec2(float(i) / float(n), radicalInverse_VdC(i));
}

vec3 sample_hemisphere_uniform(vec2 uv) {
    float phi = uv.y * 2.0 * PI;
    float cosTheta = 1.0 - uv.x;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}
   
vec3 sample_hemisphere_cos(vec2 uv) {
    float phi = uv.y * 2.0 * PI;
    float cosTheta = sqrt(1.0 - uv.x);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
vec3 sample_GGX(vec2 uv, float roughness) {
    float alpha = roughness * roughness;

    float cosTheta = sqrt((1.0 - uv.y) / (1.0 + (alpha * alpha - 1.0) * uv.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float phi = 2.0 * PI * uv.x;

    return vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// https://github.com/Nadrin/PBR/blob/7a31ffa103542c1e635b6df956c32f8f86273c55/data/shaders/glsl/irmap_cs.glsl
mat3 compute_tangent_basis(vec3 N) {
    // Branchless select non-degenerate T.
    vec3 T = cross(N, vec3(0.0, 1.0, 0.0));
    T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(0.00001, dot(T, T)));
    T = normalize(T);
    return mat3(normalize(cross(N, T)), T, N);
}
