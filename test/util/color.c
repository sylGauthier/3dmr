#include "linear_algebra.h"

void hsv2rgb(double h, double s, double v, Vec3 dest) {
    double p, q, t, rem;
    long i;

    if (s == 0.0) {
        dest[0] = v;
        dest[1] = v;
        dest[2] = v;
    } else {
        h /= 60.0;
        i = (long)h;
        rem = h - i;
        p = v * (1.0 - s);
        q = v * (1.0 - (s * rem));
        t = v * (1.0 - (s * (1.0 - rem)));

        switch (i) {
            case 0: dest[0] = v; dest[1] = t; dest[2] = p; break;
            case 1: dest[0] = q; dest[1] = v; dest[2] = p; break;
            case 2: dest[0] = p; dest[1] = v; dest[2] = t; break;
            case 3: dest[0] = p; dest[1] = q; dest[2] = v; break;
            case 4: dest[0] = t; dest[1] = p; dest[2] = v; break;
            case 5: dest[0] = v; dest[1] = p; dest[2] = q; break;
        }
    }
}
