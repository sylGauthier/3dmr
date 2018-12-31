#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <game/material/solid.h>
#include <game/material/phong.h>
#include <game/material/pbr.h>
#include <game/mesh/mesh.h>
#include "args.h"
#include "color.h"
#include "texture.h"

void usage_materials(void) {
    printf(
            "Materials:\n"
            "  solid:color(r,g,b)\n"
            "  solid:texture(path)\n"
            "  phong:color(r,g,b),phong(...)\n"
            "  phong:texture(path)[,texture(normalmapPath)],phong(...)\n"
            "  pbr:color(r,g,b),metalness,roughness\n"
            "  pbr:texture(albedo),texture(metalness),texture(roughness),texture(normalmap)\n"
            "\n"
          );
}

static int parse_color(const char* s, char** end, Vec3 color, int* isMulti) {
    if (strncmp(s, "color(", 6)) {
        return 1;
    }
    s += 6;
    *isMulti = 0;
    if (!strncmp(s, "multi)", 6)) {
        s += 6; color[0] = 0; color[1] = 0; color[2] = 0; *isMulti = 1;
    } else if (!strncmp(s, "black)", 6)) {
        s += 6; color[0] = 0; color[1] = 0; color[2] = 0;
    } else if (!strncmp(s, "red)", 4)) {
        s += 4; color[0] = 1; color[1] = 0; color[2] = 0;
    } else if (!strncmp(s, "green)", 6)) {
        s += 6; color[0] = 0; color[1] = 1; color[2] = 0;
    } else if (!strncmp(s, "blue)", 5)) {
        s += 5; color[0] = 0; color[1] = 0; color[2] = 1;
    } else if (!strncmp(s, "yellow)", 7)) {
        s += 7; color[0] = 1; color[1] = 1; color[2] = 0;
    } else if (!strncmp(s, "magenta)", 8)) {
        s += 8; color[0] = 1; color[1] = 0; color[2] = 1;
    } else if (!strncmp(s, "cyan)", 5)) {
        s += 5; color[0] = 0; color[1] = 1; color[2] = 1;
    } else if (!strncmp(s, "white)", 6)) {
        s += 6; color[0] = 1; color[1] = 1; color[2] = 1;
    } else if (!parse_vec3(s, (char**)&s, color) || *s++ != ')') {
        fprintf(stderr, "Error: failed to parse material color\n");
        return 2;
    }
    *end = (char*)s;
    return 0;
}

static int parse_texture(const char* s, char** end, GLuint* texture) {
    char *name, *e;

    if (strncmp(s, "texture(", 8)) {
        return 1;
    }
    s += 8;
    if (!(e = strchr(s, ')'))) {
        fprintf(stderr, "Error: failed to parse texture name (solid texture material)\n");
        return 2;
    }
    if (!(name = malloc(e - s + 1))) {
        fprintf(stderr, "Error: failed to allocate memory for texture name (solid texture material)\n");
        return 2;
    }
    memcpy(name, s, e - s);
    name[e - s] = 0;
    s = e + 1;
    *texture = test_texture(name);
    free(name);
    if (!*texture) {
        return 2;
    }
    *end = (char*)s;
    return 0;
}

static int parse_phong(const char* s, char** end, struct PhongMaterial* mat) {
    if (strncmp(s, "phong(", 6)) {
        return 1;
    }
    s += 6;
    if (!strncmp(s, "default)", 8)) {
        mat->ambient[0] = mat->ambient[1] = mat->ambient[2] = 0.1;
        mat->diffuse[0] = mat->diffuse[1] = mat->diffuse[2] = 0.6;
        mat->specular[0] = mat->specular[1] = mat->specular[2] = 0.1;
        mat->shininess = 1.0;
        s += 8;
    } else if (!strncmp(s, "default_no_spec)", 16)) {
        mat->ambient[0] = mat->ambient[1] = mat->ambient[2] = 0.1;
        mat->diffuse[0] = mat->diffuse[1] = mat->diffuse[2] = 0.6;
        mat->specular[0] = mat->specular[1] = mat->specular[2] = 0.0;
        mat->shininess = 1.0;
        s += 16;
    } else {
        if (!parse_vec3(s, (char**)&s, mat->ambient) || *s++ != ',') {
            fprintf(stderr, "Error: failed to parse phong() parameters\n");
            return 2;
        }
        if (parse_vec3(s, (char**)&s, mat->diffuse)) {
            if (*s++ != ',' || !parse_vec3(s, (char**)&s, mat->specular) || *s++ != ',') {
                fprintf(stderr, "Error: failed to parse phong() parameters\n");
                return 2;
            }
        } else {
            mat->diffuse[0] = mat->diffuse[1] = mat->diffuse[2] = mat->ambient[1];
            mat->specular[0] = mat->specular[1] = mat->specular[2] = mat->ambient[2];
            mat->ambient[1] = mat->ambient[2] = mat->ambient[0];
        }
        if (!parse_float(s, (char**)&s, &mat->shininess) || *s++ != ')') {
            fprintf(stderr, "Error: failed to parse phong() parameters\n");
            return 2;
        }
    }
    *end = (char*)s;
    return 0;
}

static Vec3 multiColor;
static double hue = 0.0;
static void solid_multicolor_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "solidColor"), 1, multiColor);
}
static void phong_multicolor_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "color"), 1, multiColor);
    light_load_uniforms(material->shader, lights);
    phong_material_load_uniform(material->shader, &((const struct PhongColorMaterial*)material)->phong);
}
static void pbr_multicolor_prerender(const struct Material* material, const struct Camera* camera, const struct Lights* lights) {
    glUniform3fv(glGetUniformLocation(material->shader, "color"), 1, multiColor);
    glUniform1fv(glGetUniformLocation(material->shader, "metalness"), 1, &((const struct PBRUniMaterial*)material)->metalness);
    glUniform1fv(glGetUniformLocation(material->shader, "roughness"), 1, &((const struct PBRUniMaterial*)material)->roughness);
    light_load_uniforms(material->shader, lights);
}
void update_materials(double dt) {
    hue = fmod(hue + 50.0 * dt, 360.0);
    hsv2rgb(hue, 1.0, 1.0, multiColor);
}

int parse_material(const char* s, char** end, struct Material** m, unsigned int* requiredMeshFlags) {
    struct PhongMaterial phongMat;
    Vec3 color;
    GLuint texture, normalMap;
    int res, resNormalMap, isMulti;

    if (!strncmp(s, "solid:", 6)) {
        s += 6;
        if (!(res = parse_color(s, (char**)&s, color, &isMulti))) {
            *m = (struct Material*)solid_color_material_new(color[0], color[1], color[2]);
            if (isMulti && *m) {
                (*m)->prerender = solid_multicolor_prerender;
            }
            if (requiredMeshFlags) {
                *requiredMeshFlags = 0;
            }
        } else if (res == 1 && !(res = parse_texture(s, (char**)&s, &texture))) {
            *m = (struct Material*)solid_texture_material_new(texture);
            if (requiredMeshFlags) {
                *requiredMeshFlags = MESH_TEXCOORDS;
            }
        } else {
            if (res == 1) {
                fprintf(stderr, "Error: expected 'color(' or 'texture(' (solid material)\n");
            }
            return 0;
        }
    } else if (!strncmp(s, "phong:", 6)) {
        s += 6;
        if (!(res = parse_color(s, (char**)&s, color, &isMulti))) {
            res = 1;
            if (*s++ != ',' || (res = parse_phong(s, (char**)&s, &phongMat))) {
                if (res == 1) {
                    fprintf(stderr, "Error: expected 'phong(' (phong color material)\n");
                }
                return 0;
            }
            *m = (struct Material*)phong_color_material_new(color[0], color[1], color[2], &phongMat);
            if (isMulti && *m) {
                (*m)->prerender = phong_multicolor_prerender;
            }
            if (requiredMeshFlags) {
                *requiredMeshFlags = MESH_NORMALS;
            }
        } else if (res == 1 && !(res = parse_texture(s, (char**)&s, &texture))) {
            if (*s++ != ',') {
                fprintf(stderr, "Error: expected 'phong(' (phong texture material)\n");
                return 0;
            }
            if ((resNormalMap = parse_texture(s, (char**)&s, &normalMap)) == 2) {
                return 0;
            }
            resNormalMap = !resNormalMap;
            res = 1;
            if ((resNormalMap && *s++ != ',') || (res = parse_phong(s, (char**)&s, &phongMat))) {
                if (res == 1) {
                    fprintf(stderr, "Error: expected 'phong(' (phong texture material)\n");
                }
                return 0;
            }
            if (resNormalMap) {
                *m = (struct Material*)phong_texture_normalmap_material_new(texture, normalMap, &phongMat);
            } else {
                *m = (struct Material*)phong_texture_material_new(texture, &phongMat);
            }
            if (requiredMeshFlags) {
                *requiredMeshFlags = resNormalMap ? (MESH_NORMALS | MESH_TEXCOORDS | MESH_TANGENTS) : (MESH_NORMALS | MESH_TEXCOORDS);
            }
        } else {
            if (res == 1) {
                fprintf(stderr, "Error: expected 'color(' or 'texture(' (phong material)\n");
            }
            return 0;
        }
    } else if (!strncmp(s, "pbr:", 4)) {
        s += 4;
        if (!(res = parse_color(s, (char**)&s, color, &isMulti))) {
            Vec2 params;
            if (*s++ != ',' || !parse_vec2(s, (char**)&s, params)) {
                fprintf(stderr, "Error: expected 'metalness,roughness' (PBR color material)\n");
                return 0;
            }
            *m = (struct Material*)pbr_uni_material_new(color[0], color[1], color[2], params[0], params[1]);
            if (isMulti && *m) {
                (*m)->prerender = pbr_multicolor_prerender;
            }
            if (requiredMeshFlags) {
                *requiredMeshFlags = MESH_NORMALS;
            }
        } else if (res == 1 && !(res = parse_texture(s, (char**)&s, &texture))) {
            GLuint tex[3];
            unsigned int i;
            for (i = 0; i < 3; i++) {
                if (*s++ != ',' || (res = parse_texture(s, (char**)&s, &tex[i]))) {
                    fprintf(stderr, "Error: expected 'texture(' (PBR texture material)\n");
                    return 0;
                }
            }
            *m = (struct Material*)pbr_material_new(texture, tex[2], tex[0], tex[1]);
            if (requiredMeshFlags) {
                *requiredMeshFlags = MESH_NORMALS | MESH_TEXCOORDS | MESH_TANGENTS;
            }
        } else {
            if (res == 1) {
                fprintf(stderr, "Error: expected 'color(' or 'texture(' (PBR material)\n");
            }
            return 0;
        }
    } else {
        fprintf(stderr, "Error: unknown material type\n");
        return 0;
    }

    if (!*m) {
        return 0;
    }
    if (end) {
        *end = (char*)s;
    }
    return 1;
}
