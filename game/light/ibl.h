#include <GL/glew.h>

#ifndef LIGHT_IBL_H
#define LIGHT_IBL_H

struct IBL {
    GLuint irradianceMap, specularMap, specularBrdf;
    GLint specularMapNumMips;
    int enabled;
};

int compute_ibl(GLuint envmap, unsigned int irrSize, unsigned int spSize, unsigned int spMips, unsigned int spBrdfSize, struct IBL* dest);

#endif
