#ifndef TDMR_MATH_RANDOM_H
#define TDMR_MATH_RANDOM_H

/* Mersenne-Twister PRNG (MT19937). */

struct MTState;
struct MTState* mt_state(void);

void mt_seed(struct MTState* state, unsigned long seed);
unsigned long mt_ulong_rand(struct MTState* state); /* random 32 bit unsigned integer */
float mt_float_rand(struct MTState* state); /* random float in [0, 1] */
double mt_double_rand(struct MTState* state); /* random double in [0, 1] */

/* Same, using a default global state. */

void random_seed(unsigned long seed);
unsigned long random_ulong(void); /* random 32 bit unsigned integer */
float random_float(void); /* random float in [0, 1] */
double random_double(void); /* random double in [0, 1] */

#endif
