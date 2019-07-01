#include "light/direct.glsl"
#include "material/phong.h"

uniform PhongMaterial material;

vec3 phong_compute_directional_light(DirectionalLight light, vec3 surfelToCamera, vec3 surfelNormal) {
    vec3 surfelToLight = -light.direction;

    float diffuseFactor = max(dot(surfelToLight, surfelNormal), 0.0);

    vec3 reflectDirection = reflect(surfelToLight, surfelNormal);
    float specularFactor = pow(max(dot(surfelToCamera, reflectDirection), 0.0), material.shininess);

    return (diffuseFactor * material.diffuse + specularFactor * material.specular) * light.color;
}

vec3 phong_compute_point_light(PointLight light, vec3 surfelToCamera, vec3 surfelPosition, vec3 surfelNormal) {
    vec3 surfelToLight = light.position - surfelPosition;
    float distance = length(surfelToLight);
    surfelToLight /= distance;

    float diffuseFactor = max(dot(surfelToLight, surfelNormal), 0.0);

    vec3 reflectDirection = reflect(surfelToLight, surfelNormal);
    float specularFactor = pow(max(dot(surfelToCamera, reflectDirection), 0.0), material.shininess);

    float attenuation = point_light_attenuation(light, distance);
    return attenuation * (diffuseFactor * material.diffuse + specularFactor * material.specular) * light.color;
}

vec3 phong(vec3 cameraPosition, vec3 surfelPosition, vec3 surfelNormal) {
    vec3 lightFactor = material.ambient * ambientLight.color;
    vec3 surfelToCamera = normalize(cameraPosition - surfelPosition);
    uint i;

    for (i = 0U; i < numDirectionalLights; i++) {
        lightFactor += phong_compute_directional_light(directionalLights[i], surfelToCamera, surfelNormal);
    }

    for (i = 0U; i < numPointLights; i++) {
        lightFactor += phong_compute_point_light(pointLights[i], surfelToCamera, surfelPosition, surfelNormal);
    }

    return lightFactor;
}
