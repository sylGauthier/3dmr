#define GLSL
#include "../src/light/phong.h"

uniform int nbDirectionalLights;
uniform int nbLocalLights;
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform LocalLight localLights[MAX_LOCAL_LIGHTS];
uniform PhongMaterial material;

vec3 phong_compute_directional_light(DirectionalLight light, vec3 surfelToCamera, vec3 surfelNormal) {
    vec3 surfelToLight = -light.direction;

    float diffuseFactor = max(dot(surfelToLight, surfelNormal), 0.0);
    vec3 reflectDirection = reflect(surfelToLight, surfelNormal);
    float specularFactor = pow(max(dot(surfelToCamera, reflectDirection), 0.0), material.shininess);

    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = diffuseFactor * light.diffuse * material.diffuse;
    vec3 specular = specularFactor * light.specular * material.specular;

    return (ambient + diffuse + specular);
}

vec3 phong_compute_local_light(LocalLight light, vec3 surfelToCamera, vec3 surfelPosition, vec3 surfelNormal) {
    // Diffuse shading
    vec3 surfelToLight = light.position - surfelPosition;
    float distance = length(surfelToLight);
    surfelToLight *= float(1) / distance;     // normalization
    float diffuseFactor = max(dot(surfelNormal, surfelToLight), 0.0);

    // Specular shading
    vec3 reflectDirection = reflect(-surfelToLight, surfelNormal);
    float specularFactor = (dot(surfelNormal, surfelToLight) > 0) ? pow(max(dot(surfelToCamera, reflectDirection), 0.0), material.shininess) : 0;

    float t = light.decay * distance;
    float attenuation = light.intensity / (1 + t + t * t / 2 + t * t * t / 6);

    // Combine results
    vec3 ambient  = attenuation *                  light.ambient  * material.ambient ;
    vec3 diffuse  = attenuation * diffuseFactor  * light.diffuse  * material.diffuse ;
    vec3 specular = attenuation * specularFactor * light.specular * material.specular;

    return (ambient + diffuse + specular);
}

vec3 phong_light_factor(vec3 cameraPosition, vec3 surfelPosition, vec3 surfelNormal) {
    vec3 lightFactor = vec3(0.0, 0.0, 0.0);
    vec3 surfelToCamera = normalize(cameraPosition - surfelPosition);
    int i;

    for (i = 0; i < nbDirectionalLights; i++) {
        lightFactor += phong_compute_directional_light(directionalLights[i], surfelToCamera, surfelNormal);
    }

    for (i = 0; i < nbLocalLights; i++) {
        lightFactor += phong_compute_local_light(localLights[i], surfelToCamera, surfelPosition, surfelNormal);
    }

    return lightFactor;
}
