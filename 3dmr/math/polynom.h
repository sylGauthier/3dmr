#ifndef TDMR_MATH_POLYNOM_H
#define TDMR_MATH_POLYNOM_H

/* polynom_roots_<n>, where 0 <= n <= 3:
 * - compute the roots of P(x) = sum[0 <= i <= n](coefs[i] * x^i)
 *   (note that coefs[0] is the constant term and coefs[n] is the x^n term)
 * - returns the number of roots, or -1 for infinity (equation 0 = 0)
 *   (return value r is such as -1 <= r <= n)
 * Computed roots are stored in roots[i] for 0 <= i < r.
 * Note that polynom_roots_0 returns either -1 or 0, so the roots argument is unused.
 */
int polynom_roots_0(float* unused, const float coefs[1]);
int polynom_roots_1(float roots[1], const float coefs[2]);
int polynom_roots_2(float roots[2], const float coefs[3]);
int polynom_roots_3(float roots[3], const float coefs[4]);

extern const int (*polynom_roots[4])(float* roots, const float* coefs);

#endif
