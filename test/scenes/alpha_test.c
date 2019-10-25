#include <game/material/alpha.h>
#include <game/render/texture.h>
#include "solid_color.h"

int alpha_test_setup(struct Scene* scene) {
    struct AlphaParams* params;
    GLuint alphamap;
    if (!(alphamap = texture_load_from_png("assets/grad.png"))) return 0;
    if (!solid_color_setup(scene, 2.0165, ALPHA_ENABLED | ALPHA_TEXTURED | ALPHA_TEST)) {
        glDeleteTextures(1, &alphamap);
        return 0;
    }
    params = &((struct SolidMaterial*)scene->root.children[0]->data.geometry->material)->alpha;
    params->enabled = 1;
    material_param_set_float_texture(&params->alpha, alphamap);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    return 1;
}

void alpha_test_teardown(struct Scene* scene) {
    struct AlphaParams* params = &((struct SolidMaterial*)scene->root.children[0]->data.geometry->material)->alpha;
    glDeleteTextures(1, &params->alpha.value.texture);
}
