#ifndef DEMO_UTIL_MATERIALS_H
#define DEMO_UTIL_MATERIALS_H

void usage_materials(void);

void update_materials(double dt);

int parse_material(const char* s, char** end, struct Material** m, unsigned int* requiredMeshFlags);

#endif
