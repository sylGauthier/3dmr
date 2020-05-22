#include <GL/glew.h>

#ifndef TDMR_LIGHT_IBL_H
#define TDMR_LIGHT_IBL_H

struct IBL {
    GLuint irradianceMap, specularMap, specularBrdf;
    GLint specularMapNumMips;
    int enabled;
};

int compute_ibl(GLuint envmap, unsigned int irrSize, unsigned int spSize, unsigned int spMips, unsigned int spBrdfSize, struct IBL* dest);
int compute_ibl_32(GLuint envmap, unsigned int irrSize, unsigned int spSize, unsigned int spMips, unsigned int spBrdfSize, struct IBL* dest);

void light_load_ibl_uniforms(GLuint shader, const struct IBL* ibl, unsigned int tex1, unsigned int tex2, unsigned int tex3);

#endif
