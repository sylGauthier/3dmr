#ifndef TDMR_OPENGEX_AXIS_H
#define TDMR_OPENGEX_AXIS_H

enum OgexAxis {
    AXIS_X = 1,
    AXIS_MINUS_X,
    AXIS_Y,
    AXIS_MINUS_Y,
    AXIS_Z,
    AXIS_MINUS_Z
};

enum OgexAxis ogex_parse_axis(const char* str);

#endif
