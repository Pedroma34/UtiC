#include <UtiC/core/types.h>

typedef struct Randy {
    usz state[4];
    usz seed;
} Randy;

Randy randy_init(usz seed);


