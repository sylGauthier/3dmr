#include <jansson.h>

#include "gltf.h"

enum TreeError {
    TREE_NO_ERROR = 0,
    TREE_CYCLE,
    TREE_MULTIPLE_PARENTS,
    TREE_OTHER
};

struct TreeTopology {
    unsigned int depth;
    unsigned int parent;
};

static int incr_children(struct TreeTopology* topo, unsigned int numNodes, json_t* nodes,
                         unsigned int curIdx, unsigned int curDepth, unsigned int parent) {
    json_t* children;

    if (curIdx >= numNodes) return TREE_OTHER;
    if (topo[curIdx].parent < numNodes && topo[curIdx].parent != parent) return TREE_MULTIPLE_PARENTS;
    if (curDepth >= numNodes) return TREE_CYCLE;

    topo[curIdx].parent = parent;
    topo[curIdx].depth = curDepth;
    if ((children = json_object_get(json_array_get(nodes, curIdx), "children"))) {
        unsigned int c;
        json_t* val;

        json_array_foreach(children, c, val) {
            unsigned int idx = json_integer_value(val);
            int ret;

            if ((ret = incr_children(topo, numNodes, nodes, idx, curDepth + 1, curIdx))) return ret;
        }
    }
    return TREE_NO_ERROR;
}

static struct TreeTopology* get_topology(json_t* nodes, enum TreeError* error) {
    struct TreeTopology* res;
    unsigned int numNodes, i;

    *error = TREE_NO_ERROR;
    if (!(numNodes = json_array_size(nodes))) return NULL;
    if (!(res = calloc(numNodes, sizeof(*res)))) {
        *error = TREE_OTHER;
        return NULL;
    }
    for (i = 0; i < numNodes; i++) res[i].parent = numNodes;
    for (i = 0; i < numNodes; i++) {
        if (!res[i].depth) {
            int ret;
            if ((ret = incr_children(res, numNodes, nodes, i, 0, numNodes))) {
                free(res);
                *error = ret;
                return NULL;
            }
        }
    }
    return res;
}

int gltf_get_root_nodes(json_t* nodes, unsigned int** roots, unsigned int* numRoots) {
    struct TreeTopology* topo;
    enum TreeError error;
    unsigned int numNodes, i, cnt = 0;

    if (!(topo = get_topology(nodes, &error))) {
        switch (error) {
            case TREE_NO_ERROR:
                return 1;
            case TREE_CYCLE:
                fprintf(stderr, "Error: gltf: cycle detected in node tree\n");
                return 0;
            case TREE_MULTIPLE_PARENTS:
                fprintf(stderr, "Error: gltf: a node has multiple parents\n");
                return 0;
            case TREE_OTHER:
                fprintf(stderr, "Error: gltf: unknown error regarding tree topology\n");
                return 0;
        }
    }
    *numRoots = 0;
    numNodes = json_array_size(nodes);
    for (i = 0; i < numNodes; i++) {
        if (topo[i].depth == 0) {
            (*numRoots)++;
        }
    }
    if (!(*roots = malloc(*numRoots * sizeof(**roots)))) {
        fprintf(stderr, "Error: gltf: can't allocate memory for root nodes\n");
        free(topo);
        return 0;
    }
    for (i = 0; i < numNodes; i++) {
        if (topo[i].depth == 0) {
            (*roots)[cnt] = i;
            cnt++;
        }
    }
    free(topo);
    return 1;
}
