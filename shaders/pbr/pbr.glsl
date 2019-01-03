// Adapted from https://github.com/Nadrin/PBR/blob/7a31ffa103542c1e635b6df956c32f8f86273c55/data/shaders/glsl/pbr_fs.glsl

#include "../light/direct.glsl"
#include "../light/ibl.glsl"
#include "util.glsl"

#define Fdielectric vec3(0.04)

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
    vec3 specularBRDF = (F * D * G) / max(0.00001, 4.0 * cosLi * cosLo); // Cook-Torrance specular microfacet BRDF
    return (diffuseBRDF + specularBRDF) * Lradiance * cosLi; 
}

vec3 pbr_ambient_lighting(vec3 Lr, float cosLo, vec3 N, vec3 albedo, float metalness, float roughness, vec3 F0) {
    if (hasIBL == 0) {
        return vec3(0);
    }

    // Sample diffuse irradiance at normal direction.
    vec3 irradiance = texture(irradianceMap, N).rgb;

    // Calculate Fresnel term for ambient lighting.
    // Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
    // use cosLo instead of angle with light's half-vector (cosLh above).
    // See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
    vec3 F = fresnelSchlick(F0, cosLo);

    // Get diffuse contribution factor (as with direct lighting).
    vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

    // Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
    vec3 diffuseIBL = kd * albedo * irradiance;

    // Sample pre-filtered specular reflection environment at correct mipmap level.
    vec3 specularIrradiance = textureLod(specularMap, Lr, roughness * float(specularMapNumMipmaps - 1)).rgb;

    // Split-sum approximation factors for Cook-Torrance specular BRDF.
    vec2 specularBRDF = texture(specularBrdf, vec2(cosLo, roughness)).rg;

    // Total specular IBL contribution.
    vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

    // Total ambient lighting contribution.
    return diffuseIBL + specularIBL;
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
    vec3 ambientLighting = pbr_ambient_lighting(Lr, cosLo, surfelNormal, albedo, metalness, roughness, F0);

    return ambientLighting + directLighting;
}
