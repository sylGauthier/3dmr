#include <game/material/alpha.h>
#include "solid_color.h"

int alpha_test_setup(struct Scene* scene) {
    return solid_color_setup(scene, 2.0165, ALPHA_TEST);
}

void alpha_test_teardown(struct Scene* scene) {
    struct AlphaParams* params = &((struct SolidMaterialParams*)scene->root.children[0]->data.geometry->material->fparams)->alpha;
    glDeleteTextures(1, &params->alpha.value.texture);
}
