#include <math.h>
#include <3dmr/math/constants.h>
#include <3dmr/math/interp.h>

int polynom_roots_0(float* unused, const float coefs[1]) {
    if (coefs[0] == 0.0f) {
        return -1;
    }
    return 0;
}

int polynom_roots_1(float roots[1], const float coefs[2]) {
    if (coefs[1] == 0.0f) {
        return polynom_roots_0(roots, coefs);
    }
    roots[0] = -coefs[0] / coefs[1];
    return 1;
}

int polynom_roots_2(float roots[2], const float coefs[3]) {
    float a, b, c, delta;

    if (coefs[2] == 0.0f) {
        return polynom_roots_1(roots, coefs);
    }
    a = coefs[2];
    b = coefs[1];
    c = coefs[0];
    delta = b * b - 4.0f * a * c;
    if (delta > 0.0f) {
        delta = sqrt(delta);
        a *= 2.0f;
        roots[0] = (-b - delta) / a;
        roots[1] = (-b + delta) / a;
        return 2;
    }
    if (delta == 0.0f) {
        roots[0] = -b / (2.0f * a);
        return 1;
    }
    return 0;
}

/* https://en.wikipedia.org/wiki/Cubic_equation#General_cubic_formula
 * https://fr.wikipedia.org/wiki/%C3%89quation_cubique#Expression_g%C3%A9n%C3%A9rale_des_solutions */
int polynom_roots_3(float roots[3], const float coefs[4]) {
    float a, b, bsq, c, d, delta, delta0, delta1, delta0cb, delta1sq;
    int numRoots;

    if ((a = coefs[3]) == 0.0f) {
        return polynom_roots_2(roots, coefs);
    }
    b = coefs[2] / a;
    c = coefs[1] / a;
    d = coefs[0] / a;
    bsq = b * b;
    delta0 = bsq - 3.0f * c; /* b² - 3ac, a=1 */
    delta1 = b * (2.0f * bsq - 9.0f * c) + 27.0f * d; /* 2b³ - 9abc + 27a²d = b(2b² - 9ac) + 27a²d, a=1 */
    delta0cb = delta0 * delta0 * delta0;
    delta1sq = delta1 * delta1;
    delta = 4.0f * delta0cb - delta1sq;
    if (delta > 0.0f) { /* Three distinct real roots */
        /* In this case, we don't use the general formula, since it involves complex numbers (sqrt(-delta), delta > 0).
         * We use the trigonometric solution instead. https://en.wikipedia.org/wiki/Cubic_equation#Trigonometric_solution_for_three_real_roots
         */
        float C = 2.0f * sqrt(delta0 / 9.0f);
        float t = acos(clamp(-delta1 / (3.0f * delta0 * C), -1.0f, 1.0f));
        float k = -b / 3.0f;
        roots[0] = C * cos(t / 3.0f) + k;
        roots[1] = C * cos((t + 2.0f * ((float)M_PI)) / 3.0f) + k;
        roots[2] = C * cos((t - 2.0f * ((float)M_PI)) / 3.0f) + k;
        numRoots = 3;
    } else if (delta == 0.0f) { /* One or two distinct real roots (either a double or a triple root) */
        /* In this case, we use the simplified expresions, taken from
         * https://fr.wikipedia.org/wiki/%C3%89quation_cubique#Expression_g%C3%A9n%C3%A9rale_des_solutions */
        if (delta0 == 0.0f) { /* One triple root */
            roots[0] = -b / 3.0f; /* -b / 3a, a=1 */
            numRoots = 1;
        } else { /* One double root + one single root */
            roots[0] = (9.0f * d - b * c) / (2.0f * delta0); /* 9ad - bc / 2D0, a=1 (double root) */
            roots[1] = (b * (4.0f * c - bsq) - 9.0f * d) / delta0 ; /* (4abc - 9a²d - b³) / aD0, a=1 (single root) */
            numRoots = 2;
        }
    } else { /* One real root and two complex conjugate roots */
        float C = (delta1 + sqrt(-delta)) / 2.0f;
        if (C < 0.0f) {
            C = -pow(-C, 1.0 / 3.0);
        } else {
            C = pow(C, 1.0 / 3.0);
        }
        roots[0] = -(b + C + delta0 / C) / 3.0f; /* (-1/3a) (b + C + D0/C), a=1 */
        numRoots = 1;
    }
    return numRoots;
}

int (*const polynom_roots[4])(float* roots, const float* coefs) = {
    polynom_roots_0,
    polynom_roots_1,
    polynom_roots_2,
    polynom_roots_3
};
