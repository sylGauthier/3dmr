#define GLSL

#include "ambient_light.h"
#include "directional_light.h"
#include "point_light.h"

uniform int numDirectionalLights;
uniform int numPointLights;
uniform AmbientLight ambientLight;
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
