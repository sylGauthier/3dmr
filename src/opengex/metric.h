#include <liboddl/liboddl.h>
#include "context.h"

#ifndef TDMR_OPENGEX_METRIC_H
#define TDMR_OPENGEX_METRIC_H

int ogex_parse_metric(struct OgexContext* context, const struct ODDLStructure* cur);
int ogex_parse_metrics(struct OgexContext* context, const struct ODDLStructure* root);

#endif
