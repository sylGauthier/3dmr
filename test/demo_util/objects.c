#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <game/skybox.h>
#include <game/mesh/box.h>
#include <game/mesh/icosphere.h>
#include <game/mesh/obj.h>
#include <game/mesh/sphere_uv.h>
#include "args.h"
#include "asset_manager.h"
#include "object_args.h"
#include "texture.h"
#include "materials.h"

void usage_objects(void) {
    printf(
            "Objects:\n"
            "  box:width,height,depth\n"
            "  cube:size\n"
            "  icosphere:radius[,subdiv=4][,uv=type,w,h,r]\n"
            "    type=cylindric|mercator|miller|equirect\n"
            "  mesh:objPath\n"
            "  grid:rows,cols,spacing,object        grid with the same object duplicated\n"
            "  skybox:size,lt,rt,bm,tp,bk,ft        skybox (6 last args are textures)\n"
            "         (only once, not followed by any material and object arg)\n"
            "\n"
          );
}

static int parse_skybox(const char* s, struct Config* config) {
    char* paths[6];
    char *tmp, *ptr;
    float size;
    GLuint texture = 0;
    unsigned int i;
    int ok, hdr;

    if (config->skybox) {
        fprintf(stderr, "Warning: skybox already loaded, ignoring\n");
        return 1;
    }
    if (!(tmp = malloc(strlen(s) + 1))) {
        return 0;
    }
    strcpy(tmp, s);
    size = strtod(tmp, &ptr);
    if (tmp == ptr || *ptr != ',') {
        fprintf(stderr, "Error: invalid skybox parameter\n");
        free(tmp);
        return 0;
    }
    for (i = 0; i < 6; i++) {
        paths[i] = ++ptr;
        if (!(ptr = strchr(ptr, ','))) {
            if (i == 0) {
                break;
            } else if (i != 5) {
                fprintf(stderr, "Error: invalid skybox parameter\n");
                free(tmp);
                return 0;
            }
        }
        if (ptr) {
            *ptr = 0;
        }
    }
    hdr = (i == 0);
    ok = 1;
    for (i = 0; i < (hdr ? 1 : 6); i++) {
        ok &= (paths[i] = test_texture_path(paths[i], hdr ? "hdr" : "png")) != NULL;
    }
    free(tmp);
    if (ok) {
        if (hdr) {
            texture = skybox_load_texture_hdr_equirect(paths[0], 1024);
        } else {
            texture = skybox_load_texture_png_6faces(paths[0], paths[1], paths[2], paths[3], paths[4], paths[5]);
        }
    }
    for (i = 0; i < (hdr ? 1 : 6); i++) {
        free(paths[i]);
    }
    if (!ok || !texture) {
        if (ok && !texture) {
            fprintf(stderr, "Error: failed to load skybox texture\n");
        }
        return 0;
    }

    config->skybox = skybox_new(texture, size);
    if (!config->skybox) {
        fprintf(stderr, "Error: failed to create the skybox\n");
        return 0;
    }
    {
        GLint size;
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &size);
        if (size < 256) {
            size = 256;
        } else if (size > 1024) {
            size = 1024;
        }
        printf("Info: pre-computing IBL textures... ");
        fflush(stdout);
        if (compute_ibl(texture, 32, size, 5, 256, &config->scene.lights.ibl)) {
            printf("done\n");
        } else {
            printf("fail\n");
        }
    }
    return 1;
}

int parse_mesh(const char* s, char** e, struct Mesh* mesh) {
    if (!strncmp(s, "box:", 4)) {
        Vec3 dims;
        if (!parse_vec3(s + 4, (char**)&s, dims) || dims[0] < 0.0 || dims[1] < 0.0 || dims[2] < 0.0) {
            fprintf(stderr, "Error: invalid box dimensions\n");
            return 0;
        }
        if (!make_box(mesh, dims[0], dims[1], dims[2])) {
            fprintf(stderr, "Error: failed to create box mesh\n");
            return 0;
        }
    } else if (!strncmp(s, "cube:", 5)) {
        float size;
        if (!parse_float(s + 5, (char**)&s, &size) || size < 0.0) {
            fprintf(stderr, "Error: invalid cube size\n");
            return 0;
        }
        if (!make_box(mesh, size, size, size)) {
            fprintf(stderr, "Error: failed to create cube mesh\n");
            return 0;
        }
    } else if (!strncmp(s, "icosphere:", 10)) {
        float radius;
        unsigned int subdiv = 4;
        enum SphereMapType type = NUM_SPHERE_MAP;
        Vec3 uvParams;
        if (!parse_float(s + 10, (char**)&s, &radius) || radius < 0.0) {
            fprintf(stderr, "Error: invalid icosphere radius\n");
            return 0;
        }
        if (*s == ',') {
            subdiv = strtoul(s + 1, (char**)&s, 10);
            if (subdiv < 2) {
                subdiv = 2;
                fprintf(stderr, "Warning: using subdiv=2\n");
            }
        }
        if (*s == ',') {
            s++;
            if (!strncmp(s, "cylindric,", 10)) {
                type = SPHERE_MAP_CENTRAL_CYLINDRICAL;
                s += 10;
            } else if (!strncmp(s, "mercator,", 9)) {
                type = SPHERE_MAP_MERCATOR;
                s += 9;
            } else if (!strncmp(s, "miller,", 7)) {
                type = SPHERE_MAP_MILLER;
                s += 7;
            } else if (!strncmp(s, "equirect,", 9)) {
                type = SPHERE_MAP_EQUIRECTANGULAR;
                s += 9;
            } else {
                fprintf(stderr, "Error: invalid sphere UV type\n");
                return 0;
            }
            if (!parse_vec3(s, (char**)&s, uvParams)) {
                fprintf(stderr, "Error: failed to parse UV params\n");
                return 0;
            }
        }
        if (!make_icosphere(mesh, radius, subdiv)) {
            fprintf(stderr, "Error: failed to create icosphere mesh\n");
            return 0;
        }
        if (type != NUM_SPHERE_MAP) {
            if (!compute_sphere_uv(mesh, uvParams[0], uvParams[1], uvParams[0], type)) {
                fprintf(stderr, "Warning: failed to create sphere UVs\n");
            }
        }
    } else if (!strncmp(s, "mesh:", 5)) {
        char *path, *end, *realpath;
        s += 5;
        if (!(end = strchr(s, ';'))) {
            fprintf(stderr, "Error: expected ';material'\n");
            return 0;
        }
        if (!(path = malloc(end - s + 12))) {
            fprintf(stderr, "Error: failed to allocate memory for mesh path\n");
            return 0;
        }
        memcpy(path, "meshes/", 7);
        memcpy(path + 7, s, end - s);
        memcpy(path + 7 + (end - s), ".obj", 5);
        realpath = asset_manager_find_file(path);
        if (!realpath) {
            fprintf(stderr, "Error: failed to find mesh '%s'\n", path + 7);
        }
        free(path);
        if (!realpath) {
            return 0;
        }
        s = end;
        if (!make_obj(mesh, realpath, 0, 1, 1)) {
            fprintf(stderr, "Error: failed to create OBJ mesh\n");
            free(realpath);
            return 0;
        }
        free(realpath);
    } else {
        fprintf(stderr, "Error: invalid object name\n");
        return 0;
    }
    if (e) {
        *e = (char*)s;
    }
    return 1;
}

struct Node* make_single(const struct Mesh* mesh, struct Material* material) {
    struct Node* node;
    struct GLObject* object;

    if ((node = malloc(sizeof(*node)))) {
        if ((object = malloc(sizeof(*object)))) {
            if ((object->vertexArray = vertex_array_new(mesh))) {
                object->material = material;
                node_init(node);
                node_set_geometry(node, object);
                return node;
            }
            free(object);
        }
        free(node);
    }
    return NULL;
}

struct Node* make_grid(const struct Mesh* mesh, struct Material* material, unsigned int rows, unsigned int cols, float spacing) {
    struct Node *nodes, *n;
    struct GLObject* object;
    unsigned int r, c = 0;
    Vec3 t;

    if ((nodes = malloc((1 + rows * cols) * sizeof(nodes[0])))) {
        if ((object = malloc(sizeof(*object)))) {
            if ((object->vertexArray = vertex_array_new(mesh))) {
                object->material = material;
                node_init(nodes);
                n = nodes;
                t[1] = 0;
                t[2] = -((float)(rows - 1) / 2.0f) * spacing;
                for (r = 0; r < rows; r++) {
                    t[0] = -((float)(cols - 1) / 2.0f) * spacing;
                    for (c = 0; c < cols; c++) {
                        n++;
                        node_init(n);
                        node_set_geometry(n, object);
                        node_translate(n, t);
                        if (!node_add_child(nodes, n)) {
                            break;
                        }
                        t[0] += spacing;
                    }
                    t[2] += spacing;
                }
                if (r == rows && c == cols) {
                    return nodes;
                }
                free(nodes->children);
                vertex_array_free(object->vertexArray);
            }
            free(object);
        }
        free(nodes);
    }
    return NULL;
}

int parse_object(const char* s, struct Config* config) {
    struct Mesh mesh;
    struct Material* material;
    struct Node** tmp;
    unsigned int requiredFlags;
    int isGrid = 0, gridRows, gridCols;
    float gridSpacing;

    if (!strncmp(s, "skybox:", 7)) {
        return parse_skybox(s + 7, config);
    }
    if (!strncmp(s, "grid:", 5)) {
        isGrid = 1;
        if ((gridRows = strtol(s + 5, (char**)&s, 10)) <= 0 || *s++ != ','
         || (gridCols = strtol(s, (char**)&s, 10)) <= 0 || *s++ != ','
         || !parse_float(s, (char**)&s, &gridSpacing) || gridSpacing <= 0.0 || *s++ != ','
         || !parse_mesh(s, (char**)&s, &mesh)) {
            fprintf(stderr, "Error: invalid grid parameters\n");
            return 0;
        }
    } else if (!parse_mesh(s, (char**)&s, &mesh)) {
        return 0;
    }
    if (*s++ != ';') {
        fprintf(stderr, "Error: expected ';material'\n");
        mesh_free(&mesh);
        return 0;
    }
    if (!parse_material(s, (char**)&s, &material, &requiredFlags)) {
        mesh_free(&mesh);
        return 0;
    }
    if (requiredFlags & MESH_TANGENTS) {
        if (!mesh_compute_tangents(&mesh)) {
            fprintf(stderr, "Error: failed to compute mesh tangents\n");
        }
    }
    if ((mesh.flags & requiredFlags) != requiredFlags) {
        if ((mesh.flags & MESH_NORMALS) < (requiredFlags & MESH_NORMALS)) {
            fprintf(stderr, "Error: mesh is missing normals, but selected material requires them\n");
        }
        if ((mesh.flags & MESH_TEXCOORDS) < (requiredFlags & MESH_TEXCOORDS)) {
            fprintf(stderr, "Error: mesh is missing texcoords, but selected material requires them\n");
        }
        mesh_free(&mesh);
        free(material);
        return 0;
    }
    if (!(tmp = realloc(config->objects, (config->numObjects + 1) * sizeof(struct Node*)))) {
        fprintf(stderr, "Error: failed to allocate node\n");
    } else {
        config->objects = tmp;
        tmp += config->numObjects;
        *tmp = isGrid ? make_grid(&mesh, material, gridRows, gridCols, gridSpacing) : make_single(&mesh, material);
        if (!(*tmp)) {
            fprintf(stderr, "Error: failed to create node\n");
        } else {
            config->numObjects++;
            while (*s) {
                if (*s == ';') {
                    if (parse_object_args(s + 1, (char**)&s, *tmp)) {
                        continue;
                    }
                } else if (*s == '\n') {
                    s++;
                    continue;
                }
                fprintf(stderr, "Error: invalid object arguments\n");
                break;
            }
            if (!*s && scene_add(&config->scene, *tmp)) {
                mesh_free(&mesh);
                return 1;
            }
        }
    }
    mesh_free(&mesh);
    if (tmp && *tmp) {
        config_object_free(config, config->numObjects - 1);
    } else {
        free(material);
    }
    return 0;
}
