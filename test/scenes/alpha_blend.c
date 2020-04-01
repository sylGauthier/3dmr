#include <game/material/alpha.h>
#include "solid_color.h"

int alpha_blend_setup(struct Scene* scene) {
    return solid_color_setup(scene, 2.0165, ALPHA_BLEND);
}

void alpha_blend_teardown(struct Scene* scene) {
    struct AlphaParams* params = &((struct SolidMaterialParams*)scene->root.children[0]->data.geometry->material->params)->alpha;
    glDeleteTextures(1, &params->alpha.value.texture);
}
