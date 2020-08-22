#include <string.h>

#include "gltf.h"
#include "light_node.h"
#include "topology.h"

static int parse_cam_node(struct GltfContext* context, struct Node* node, json_t* jcam) {
    unsigned int idx;

    node->type = NODE_CAMERA;
    import_add_metadata_camnode(context->metadata, node);
    idx = json_integer_value(jcam);
    if (idx < context->numCameras) {
        node->data.camera = context->cameras[idx];
    } else {
        fprintf(stderr, "Error: gltf: camera node: invalid camera index\n");
        return 0;
    }
    return 1;
}

static int parse_mesh_node(struct GltfContext* context, struct Node* node, json_t* jnode) {
    json_t *jmesh;
    unsigned int meshIdx;

    node->type = NODE_GEOMETRY;
    if (!(jmesh = json_object_get(jnode, "mesh"))) return 0;
    meshIdx = json_integer_value(jmesh);
    if (!json_is_integer(jmesh) || meshIdx >= context->numMeshes) {
        fprintf(stderr, "Error: gltf: node: invalid mesh index\n");
        return 0;
    }
    node_set_geometry(node, context->meshes[meshIdx]);
    return 1;
}

static int parse_node_type(struct GltfContext* context, struct Node* node, json_t* jnode) {
    json_t* tmp;

    if ((tmp = json_object_get(jnode, "mesh"))) {
        return parse_mesh_node(context, node, jnode);
    } else if ((tmp = json_object_get(jnode, "camera"))) {
        return parse_cam_node(context, node, tmp);
    } else if ((tmp = json_object_get(jnode, "extensions"))) {
        return gltf_parse_light_node(context, node, tmp);
    } else {
        node->type = NODE_EMPTY;
    }
    return 1;
}

static int parse_node(struct GltfContext* context, struct Node* node, json_t* jnode) {
    json_t* tmp;
    Quaternion rot;
    Vec3 pos, scale = {1, 1, 1};

    zero3v(pos);
    quaternion_load_id(rot);

    if ((tmp = json_object_get(jnode, "translation"))) {
        unsigned int i;

        for (i = 0; i < 3; i++) pos[i] = json_number_value(json_array_get(tmp, i));
    }
    if ((tmp = json_object_get(jnode, "rotation"))) {
        unsigned int i;

        for (i = 0; i < 4; i++) rot[(i + 1) % 4] = json_number_value(json_array_get(tmp, i));
    }
    if ((tmp = json_object_get(jnode, "scale"))) {
        unsigned int i;

        for (i = 0; i < 3; i++) scale[i] = json_number_value(json_array_get(tmp, i));
    }
    node_set_scale(node, scale);
    node_set_orientation(node, rot);
    node_set_pos(node, pos);

    if ((tmp = json_object_get(jnode, "name")) && json_string_length(tmp)) {
        if (!(node->name = malloc(json_string_length(tmp) + 1))) {
            fprintf(stderr, "Error: gltf: node: can't allocate memory for name\n");
            return 0;
        }
        strcpy(node->name, json_string_value(tmp));
    }
    if ((tmp = json_object_get(jnode, "children"))) {
        int idx;
        json_t* val;

        json_array_foreach(tmp, idx, val) {
            unsigned int child;

            child = json_integer_value(val);
            if (child < context->numNodes) { /* should always be true thanks to topo check but just to be sure */
                if (!node_add_child(node, context->nodes[child])) {
                    fprintf(stderr, "Error: gltf: node: can't add child to node\n");
                    return 0;
                }
            }
        }
    }
    return parse_node_type(context, node, jnode);
}

int gltf_parse_nodes(struct GltfContext* context, struct Node* root, json_t* jroot) {
    json_t* nodes;
    unsigned int numNodes, numRoots, i;
    unsigned int* roots;

    if (!(nodes = json_object_get(jroot, "nodes"))) {
        fprintf(stderr, "Error: gltf: node: missing nodes array\n");
        return 0;
    }
    if (!(numNodes = json_array_size(nodes))) {
        fprintf(stderr, "Error: gltf: node: no node found\n");
        return 0;
    }
    if (!gltf_get_root_nodes(nodes, &roots, &numRoots)) return 0;
    if (!(context->nodes = malloc(numNodes * sizeof(*context->nodes)))) {
        fprintf(stderr, "Error: gltf: node: can't allocate memory for node array\n");
        free(roots);
        return 0;
    }
    context->numNodes = numNodes;

    /* First allocate all nodes so we can link node i to node j when i < j */
    for (i = 0; i < numNodes; i++) {
        if (!(context->nodes[i] = malloc(sizeof(struct Node)))) {
            fprintf(stderr, "Error: gltf: node: can't allocate memory for node\n");
            free(roots);
            return 0;
        }
        node_init(context->nodes[i]);
    }

    /* Then actually parse the nodes */
    for (i = 0; i < numNodes; i++) {
        if (!parse_node(context, context->nodes[i], json_array_get(nodes, i))) {
            free(roots);
            return 0;
        }
    }

    /* Add root nodes as children of the main root node */
    for (i = 0; i < numRoots; i++) {
        if (!node_add_child(root, context->nodes[roots[i]])) {
            fprintf(stderr, "Error: gltf: node: can't add child to main node\n");
            free(roots);
            return 0;
        }
    }
    free(roots);
    return 1;
}
