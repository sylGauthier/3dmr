// Adapted from https://github.com/Nadrin/PBR/blob/7a31ffa103542c1e635b6df956c32f8f86273c55/data/shaders/glsl/pbr_fs.glsl

#include "lights.glsl"

#define Fdielectric vec3(0.04)
#define PI 3.141592
#define EPSILON 0.00001

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness) {
    float alpha   = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k) {
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta) {
    return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 pbr_direct_lighting(vec3 Li, vec3 Lo, float cosLo, vec3 N, vec3 albedo, float metalness, float roughness, vec3 F0, vec3 Lradiance) {
    vec3 Lh = normalize(Li + Lo); // Half-vector between Li and Lo
    float cosLi = max(0.0, dot(N, Li));
    float cosLh = max(0.0, dot(N, Lh));
    
    vec3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo))); // Fresnel term for direct lighting
    float D = ndfGGX(cosLh, roughness); // normal distribution for specular BRDF
    float G = gaSchlickGGX(cosLi, cosLo, roughness); // geometric attenuation for specular BRDF

    // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium
    // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero
    // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness
    vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

    vec3 diffuseBRDF = kd * albedo; // Lambert diffuse BRDF
    vec3 specularBRDF = (F * D * G) / max(EPSILON, 4.0 * cosLi * cosLo); // Cook-Torrance specular microfacet BRDF
    return (diffuseBRDF + specularBRDF) * Lradiance * cosLi; 
}

vec3 pbr(vec3 albedo, float metalness, float roughness, vec3 surfelNormal, vec3 surfelPosition, vec3 cameraPosition) {
    // Outgoing light
    vec3 Lo = normalize(cameraPosition - surfelPosition);
    float cosLo = max(0.0, dot(surfelNormal, Lo));

    // Specular reflection vector
    vec3 Lr = 2.0 * cosLo * surfelNormal - Lo;
    
    // Fresnel reflectance at normal incidence (for metals use albedo color)
    vec3 F0 = mix(Fdielectric, albedo, metalness);

    // Direct lighting
    vec3 directLighting = vec3(0);
    for (int i = 0; i < numDirectionalLights; i++) {
        vec3 Li = -directionalLights[i].direction;
        vec3 Lradiance = directionalLights[i].color;
        directLighting += pbr_direct_lighting(Li, Lo, cosLo, surfelNormal, albedo, metalness, roughness, F0, Lradiance);
    }
    for (int i = 0; i < numPointLights; i++) {
        vec3 Li = pointLights[i].position - surfelPosition;
        float distance = length(Li);
        Li /= distance;
        vec3 Lradiance = pointLights[i].color * point_light_attenuation(pointLights[i], distance);
        directLighting += pbr_direct_lighting(Li, Lo, cosLo, surfelNormal, albedo, metalness, roughness, F0, Lradiance);
    }

    // Ambient lighting
    vec3 ambientLighting = vec3(0.0);

    return ambientLighting + directLighting;
}
