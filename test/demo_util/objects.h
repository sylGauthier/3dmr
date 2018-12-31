#ifndef DEMO_UTIL_OBJECTS_H
#define DEMO_UTIL_OBJECTS_H

struct Config;

void usage_objects(void);
int parse_object(const char* s, struct Config* config);

#endif
