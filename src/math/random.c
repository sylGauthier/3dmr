#include <stdlib.h>
#include <3dmr/math/linear_algebra.h>


/* Mersenne-Twister PRNG (MT19937).
 * Implementation based on the original paper, accessed at:
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/ARTICLES/mt.pdf
 */

#define MT_N 624
#define MT_M 397
#define MT_L_MASK 0x7FFFFFFF
#define MT_U_MASK 0x80000000
#define MT_A 0x9908B0DF
#define MT_U 11
#define MT_S 7
#define MT_B 0x9D2C5680
#define MT_T 15
#define MT_C 0xEFC60000
#define MT_L 18

struct MTState {
    unsigned long x[MT_N];
    unsigned int i;
};

struct MTState* mt_state(void) {
    return malloc(sizeof(struct MTState));
}

/* Knuth 1981, The Art of Computer Programming Vol. 2, 2nd Ed., pp102, line 25 of table 1 */
void mt_seed(struct MTState* state, unsigned long seed) {
    state->x[state->i = 0] = seed & 0xFFFFFFFF;
    while (++(state->i) < MT_N) {
        state->x[state->i] = (69069 * state->x[state->i - 1]) & 0xFFFFFFFF;
    }
}

unsigned long mt_ulong_rand(struct MTState* state) {
    static unsigned long zeroOrA[] = {0, MT_A};
    unsigned long y;

    if (state->i >= MT_N) {
        unsigned int i;

        for (i = 0; i < MT_N - MT_M; i++) {
            y = (state->x[i] & MT_U_MASK) | (state->x[i + 1] & MT_L_MASK);
            state->x[i] = state->x[i + MT_M] ^ (y >> 1) ^ zeroOrA[y & 1];
        }
        for (; i < MT_N - 1; i++) {
            y = (state->x[i] & MT_U_MASK) | (state->x[i + 1] & MT_L_MASK);
            state->x[i] = state->x[i + MT_M - MT_N] ^ (y >> 1) ^ zeroOrA[y & 1];
        }
        y = (state->x[i] & MT_U_MASK) | (state->x[0] & MT_L_MASK);
        state->x[i] = state->x[MT_M - 1] ^ (y >> 1) ^ zeroOrA[y & 1];
        state->i = 0;
    }

    y = state->x[state->i++];
    y ^= (y >> MT_U);
    y ^= (y << MT_S) & MT_B;
    y ^= (y << MT_T) & MT_C;
    y ^= (y >> MT_L);

    return y;
}

float mt_float_rand(struct MTState* state) {
    return ((double)mt_ulong_rand(state)) / ((double)0xFFFFFFFF);
}

double mt_double_rand(struct MTState* state) {
    return ((double)mt_ulong_rand(state)) / ((double)0xFFFFFFFF);
}

static struct MTState mtDefault;

void random_seed(unsigned long seed) {
    mt_seed(&mtDefault, seed);
}

unsigned long random_ulong(void) {
    return mt_ulong_rand(&mtDefault);
}

float random_float(void) {
    return mt_float_rand(&mtDefault);
}

double random_double(void) {
    return mt_double_rand(&mtDefault);
}
