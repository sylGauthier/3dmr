#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <3dmr/material/phong.h>
#include <3dmr/math/utils.h>

#include "anim.h"
#include "context.h"
#include "camera_node.h"
#include "geometry_node.h"
#include "light_node.h"
#include "name.h"
#include "node.h"
#include "transform.h"

static int parse_node_transform(const struct OgexContext* context, struct Node* node, const struct ODDLStructure* cur) {
    Mat4 transform;
    Vec3 scale, pos;
    Quaternion quat;

    if (!ogex_parse_transforms(context, cur, &transform, 1)) {
        return 0;
    }
    if (!mat4toposrotscale(transform, pos, quat, scale)) {
        fprintf(stderr, "Error: parse_node_transform: invalid transform matrix (null scaling)\n");
        return 0;
    }
    node_set_scale(node, scale);
    node_set_orientation(node, quat);
    node_set_pos(node, pos);
    return 1;
}

static struct Node* ogex_parse_node(struct OgexContext* context, struct Node* parent, const struct ODDLStructure* cur) {
    unsigned int i;
    struct Node* newNode;

    if (!(newNode = malloc(sizeof(*newNode)))) {
        fprintf(stderr, "Error: Node: can't allocate memory for new node\n");
        return NULL;
    }
    node_init(newNode);
    if (!node_add_child(parent, newNode)) {
        free(newNode);
        return NULL;
    }

    for (i = 0; i < cur->nbStructures; i++) {
        struct ODDLStructure* tmp = cur->structures[i];

        if (!tmp->identifier) {
            continue;
        } else if (!strcmp(tmp->identifier, "Transform")) {
            parse_node_transform(context, newNode, tmp);
            node_update_matrices(newNode);
        } else if (!strcmp(tmp->identifier, "Translation")) {
            if (!ogex_parse_translation(context, newNode, tmp)) return 0;
        } else if (!strcmp(tmp->identifier, "Rotation")) {
            if (!ogex_parse_rotation(context, newNode, tmp)) return 0;
        } else if (!strcmp(tmp->identifier, "Scale")) {
            if (!ogex_parse_scale(context, newNode, tmp)) return 0;
        } else if (!strcmp(tmp->identifier, "Animation")) {
            if (!ogex_parse_animation(context, newNode, tmp)) return 0;
        } else if (!strcmp(tmp->identifier, "Name")) {
            free(newNode->name);
            newNode->name = ogex_parse_name(tmp);
        }
    }
    return newNode;
}

struct ParseNode {
    const struct ODDLStructure* ogex;
    struct Node* dest;
    unsigned int numDone;
};

static int push_parse_node(struct ParseNode** stack, unsigned int* stsize, const struct ODDLStructure* ogex, struct Node* dest) {
    struct ParseNode* tmp;
    if (*stsize >= ((unsigned int)-1) || !(tmp = realloc(*stack, (*stsize + 1) * sizeof(**stack)))) return 0;
    *stack = tmp;
    tmp += (*stsize)++;
    tmp->ogex = ogex;
    tmp->dest = dest;
    tmp->numDone = 0;
    return 1;
}

int ogex_parse_nodes(struct OgexContext* context, const struct ODDLStructure* docRoot, struct Node* root) {
    struct ParseNode* stack = NULL;
    unsigned int numStack = 0, numRootChildren = root->nbChildren;
    int ok = 1;

    if (!push_parse_node(&stack, &numStack, docRoot, root)) return 0;
    while (ok && numStack) {
        const struct ODDLStructure *cur = stack[numStack - 1].ogex, *tmp;
        enum NodeType type = -1;
        struct Node* node;
        while (stack[numStack - 1].numDone < cur->nbStructures) {
            tmp = cur->structures[stack[numStack - 1].numDone++];
            if (!tmp->identifier) {
                continue;
            } else if (!strcmp(tmp->identifier, "BoneNode")) {
                type = NODE_BONE;
            } else if (!strcmp(tmp->identifier, "CameraNode")) {
                type = NODE_CAMERA;
            } else if (!strcmp(tmp->identifier, "GeometryNode")) {
                type = NODE_GEOMETRY;
            } else if (!strcmp(tmp->identifier, "LightNode")) {
                type = NODE_PLIGHT;
            } else if (!strcmp(tmp->identifier, "Node")) {
                type = NODE_EMPTY;
            } else {
                continue;
            }
            break;
        }
        if (type == ((enum NodeType)-1)) {
            numStack--;
            continue;
        }
        if (!(node = ogex_parse_node(context, stack[numStack - 1].dest, tmp))
         || !push_parse_node(&stack, &numStack, tmp, node)) {
            ok = 0;
        } else switch (type) {
            case NODE_CAMERA: ok = ogex_parse_camera_node(context, node, tmp); break;
            case NODE_GEOMETRY: ok = ogex_parse_geometry_node(context, node, tmp); break;
            case NODE_PLIGHT: ok = ogex_parse_light_node(context, node, tmp); break;
            case NODE_BONE: node->type = type; ok = ogex_context_add_object(&context->boneNodes, tmp, node); break;
            default:;
        }
    }
    free(stack);
    if (!ok) {
        while (root->nbChildren > numRootChildren) {
            nodes_free(root->children[--(root->nbChildren)], imported_node_free);
        }
    }
    return ok;
}
