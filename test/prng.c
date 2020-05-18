#include <stdio.h>
#include <stdlib.h>
#include <3dmr/math/random.h>

static void usage(const char* prog) {
    printf("Usage: %s n [seed]\n", prog);
}

int main(int argc, char** argv) {
    unsigned long i, n;

    if (argc < 2 || argc > 3) {
        usage(argv[0]);
        return 1;
    }
    n = strtoul(argv[1], NULL, 10);
    if (argc == 3) {
        random_seed(strtoul(argv[2], NULL, 10));
    } else {
        random_seed(4357);
    }
    for (i = 0; i < n; i++) {
        printf("%f\n", random_double());
    }
    return 0;
}
