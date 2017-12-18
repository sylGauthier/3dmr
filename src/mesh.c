#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#include "mesh.h"

void mesh_load(struct Mesh* mesh, const char* modeldir, const char* filename)
{
    FILE* objFile;
    char* data;
    int len;
    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes;
    long unsigned int numShapes;
    tinyobj_material_t* materials;
    long unsigned int numMaterials;
    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
    unsigned int i;

    chdir(modeldir);

    if ((objFile = fopen(filename, "r")))
    {
        if (!fseek(objFile, 0, SEEK_END))
        {
            if ((data = malloc((len = ftell(objFile)))))
            {
                if (!fseek(objFile, 0, SEEK_SET) && fread(data, 1, len, objFile) == len)
                {
                    if (tinyobj_parse_obj(&attrib, &shapes, &numShapes, &materials, &numMaterials, data, len, flags) == TINYOBJ_SUCCESS)
                    {

                        /*printf("Loaded object: %s\n", filename);
                        printf("Nb shapes: %lu\nNb materials: %lu\nNb faces: %d\nNb vertices: %d\nNb normals: %d\n",
                                numShapes, numMaterials, attrib.num_faces, attrib.num_vertices, attrib.num_normals);*/

                        mesh->numVertices = attrib.num_faces;
                        if ((mesh->vertices = malloc(3 * mesh->numVertices * sizeof(float)))
                         && (mesh->normals = malloc(3 * mesh->numVertices * sizeof(float)))
                         && (mesh->texCoords = malloc(2 * mesh->numVertices * sizeof(float))))
                        {
                            for (i = 0; i < mesh->numVertices; i++)
                            {
                                mesh->vertices[3*i] = attrib.vertices[3 * attrib.faces[i].v_idx];
                                mesh->vertices[3*i + 1] = attrib.vertices[3 * attrib.faces[i].v_idx + 1];
                                mesh->vertices[3*i + 2] = attrib.vertices[3 * attrib.faces[i].v_idx + 2];

                                mesh->normals[3*i] = attrib.normals[3 * attrib.faces[i].vn_idx];
                                mesh->normals[3*i + 1] = attrib.normals[3 * attrib.faces[i].vn_idx + 1];
                                mesh->normals[3*i + 2] = attrib.normals[3 * attrib.faces[i].vn_idx + 2];
                                
                                if (i < 6*attrib.num_texcoords)
                                {
                                    mesh->texCoords[2*i] = attrib.texcoords[2 * attrib.faces[i].vt_idx];
                                    mesh->texCoords[2*i + 1] = 1 - attrib.texcoords[2 * attrib.faces[i].vt_idx + 1];
                                }
                            }

                            mesh->mat.ambient[0] = 1.0;
                            mesh->mat.ambient[1] = 1.0;
                            mesh->mat.ambient[2] = 1.0;
                            mesh->mat.diffuse[0] = 1.0;
                            mesh->mat.diffuse[1] = 1.0;
                            mesh->mat.diffuse[2] = 1.0;
                            mesh->mat.specular[0] = 1.0;
                            mesh->mat.specular[1] = 1.0;
                            mesh->mat.specular[2] = 1.0;
                            mesh->mat.shininess = 1.0;
                        }
                        else
                            fprintf(stderr, "Error: failed to allocate vertices or normals buffer\n");
                        tinyobj_attrib_free(&attrib);
                        tinyobj_shapes_free(shapes, numShapes);
                        tinyobj_materials_free(materials, numMaterials);
                    }
                    else
                        fprintf(stderr, "Error: failed to parse obj file '%s'\n", filename);
                }
                else
                    fprintf(stderr, "Error: failed to read '%s'\n", filename);
                free(data);
            }
            else
                fprintf(stderr, "Error: cannot allocate memory for obj file data\n");
        }
        else
            fprintf(stderr, "Error: failed to get file size '%s'\n", filename);
        fclose(objFile);
    }
    else
        fprintf(stderr, "Error: could not open obj file '%s'\n", filename);
}

void mesh_free(struct Mesh* mesh)
{
    if (mesh->vertices)
        free(mesh->vertices);
    if (mesh->normals)
        free(mesh->normals);
    if (mesh->texCoords)
        free(mesh->texCoords);
}

