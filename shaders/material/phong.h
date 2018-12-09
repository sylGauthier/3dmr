#ifndef PHONG_H
#define PHONG_H

struct PhongMaterial {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

#endif
