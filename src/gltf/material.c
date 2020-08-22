#include "gltf.h"

static int set_texture(struct GltfContext* context, struct MatParamVec3* param, json_t* jtex) {
    json_t* tmp;
    unsigned int texIdx;

    if (!(tmp = json_object_get(jtex, "index"))
            || !json_is_integer(tmp)
            || (texIdx = json_integer_value(tmp)) >= context->numTextures) {
        fprintf(stderr, "Error: gltf: material: invalid texture index\n");
        return 0;
    }
    material_param_set_vec3_texture(param, context->textures[texIdx]);
    return 1;
}

int gltf_parse_materials(struct GltfContext* context, json_t* jroot) {
    json_t *materials, *curMat;
    unsigned int idx;

    if (!(materials = json_object_get(jroot, "materials"))) {
        return 1;
    }
    if (!(context->numMaterials = json_array_size(materials))) {
        return 1;
    }
    if (!(context->materials = malloc(context->numMaterials * sizeof(void*)))) {
        fprintf(stderr, "Error: gltf: materials: can't allocate memory for materials\n");
        return 0;
    }
    json_array_foreach(materials, idx, curMat) {
        json_t* pbr;
        struct PBRMaterialParams* pbrMat;
        Vec3 albedo;

        if (       !(pbrMat = pbr_material_params_new())
                || !import_add_shared_matparam(context->shared, pbrMat)) {
            fprintf(stderr, "Error: gltf: material: could not create new matParam\n");
            return 0;
        }
        if ((pbr = json_object_get(curMat, "pbrMetallicRoughness"))) {
            json_t *color, *metal, *roughness;
            if ((color = json_object_get(pbr, "baseColorFactor"))) {
                albedo[0] = json_number_value(json_array_get(color, 0));
                albedo[1] = json_number_value(json_array_get(color, 1));
                albedo[2] = json_number_value(json_array_get(color, 2));
                material_param_set_vec3_constant(&pbrMat->albedo, albedo);
            } else if ((color = json_object_get(pbr, "baseColorTexture"))) {
                if (!set_texture(context, &pbrMat->albedo, color)) {
                    return 0;
                }
            }
            if ((metal = json_object_get(pbr, "metallicRoughnessTexture"))) {
                fprintf(stderr, "Warning: gltf: material: metallic/roughness texture not supported yet\n");
            }
            if ((metal = json_object_get(pbr, "metallicFactor"))) {
                material_param_set_float_constant(&pbrMat->metalness, json_number_value(metal));
            }
            if ((roughness = json_object_get(pbr, "roughnessFactor"))) {
                material_param_set_float_constant(&pbrMat->roughness, json_number_value(roughness));
            }
        }
        context->materials[idx] = pbrMat;
    }
    return 1;
}
