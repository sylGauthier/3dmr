#include "light/ambiant_light.h"
#include "light/directional_light.h"
#include "light/point_light.h"

uniform int numDirectionalLights;
uniform int numPointLights;
uniform AmbiantLight ambiantLight;
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
