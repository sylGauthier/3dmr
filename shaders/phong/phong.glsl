#include "../light/direct.glsl"

vec3 phong_compute_directional_light(vec3 diffuse, vec3 specular, float shininess, DirectionalLight light, vec3 surfelToCamera, vec3 surfelNormal) {
    vec3 surfelToLight = -light.direction;

    float diffuseFactor = max(dot(surfelToLight, surfelNormal), 0.0);

    vec3 reflectDirection = reflect(light.direction, surfelNormal);
    float specularFactor = pow(max(dot(surfelToCamera, reflectDirection), 0.0), shininess);

    return (diffuseFactor * diffuse + specularFactor * specular) * light.color;
}

vec3 phong_compute_point_light(vec3 diffuse, vec3 specular, float shininess, PointLight light, vec3 surfelToCamera, vec3 surfelPosition, vec3 surfelNormal) {
    vec3 surfelToLight = light.position - surfelPosition;
    float distance = length(surfelToLight);
    surfelToLight /= distance;

    float diffuseFactor = max(dot(surfelToLight, surfelNormal), 0.0);

    vec3 reflectDirection = reflect(-surfelToLight, surfelNormal);
    float specularFactor = pow(max(dot(surfelToCamera, reflectDirection), 0.0), shininess);

    float attenuation = point_light_attenuation(light, distance);
    return attenuation * (diffuseFactor * diffuse + specularFactor * specular) * light.color;
}

vec3 phong_compute_spot_light(vec3 diffuse, vec3 specular, float shininess, SpotLight light, vec3 surfelToCamera, vec3 surfelPosition, vec3 surfelNormal) {
    vec3 surfelToLight = normalize(light.position - surfelPosition);

    float diffuseFactor = max(dot(surfelToLight, surfelNormal), 0.0);

    vec3 reflectDirection = reflect(-surfelToLight, surfelNormal);
    float specularFactor = pow(max(dot(surfelToCamera, reflectDirection), 0.0), shininess);

    float attenuation = spot_light_attenuation(light, surfelPosition);
    return attenuation * (diffuseFactor * diffuse + specularFactor * specular) * light.color;
}

vec3 phong(vec3 ambient, vec3 diffuse, vec3 specular, float shininess, vec3 cameraPosition, vec3 surfelPosition, vec3 surfelNormal) {
    vec3 lightFactor = ambient * ambientLight.color;
    vec3 surfelToCamera = normalize(cameraPosition - surfelPosition);
    uint i;

    for (i = 0U; i < numDirectionalLights; i++) {
        lightFactor += phong_compute_directional_light(diffuse, specular, shininess, directionalLights[i], surfelToCamera, surfelNormal)
                     * (1 - compute_shadow(surfelPosition, surfelNormal, i));
    }

    for (i = 0U; i < numPointLights; i++) {
        lightFactor += phong_compute_point_light(diffuse, specular, shininess, pointLights[i], surfelToCamera, surfelPosition, surfelNormal);
    }

    for (i = 0U; i < numSpotLights; i++) {
        lightFactor += phong_compute_spot_light(diffuse, specular, shininess, spotLights[i], surfelToCamera, surfelPosition, surfelNormal);
    }

    return lightFactor;
}
