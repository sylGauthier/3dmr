#include <3dmr/light/light.h>

void light_init(struct Lights* lights) {
    lights->numDirectionalLights = 0;
    lights->numPointLights = 0;
    lights->numSpotLights = 0;
    lights->ambientLight.color[0] = 0;
    lights->ambientLight.color[1] = 0;
    lights->ambientLight.color[2] = 0;
}
