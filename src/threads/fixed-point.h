#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

//not sure if this will have to change
#define N_FRACTIONAL_BITS 14
#define FACTOR (2^N_FRACTIONAL_BITS)

#define INT_TO_FIXED(n) ((n) * FACTOR)

// fixed point number x to integer rounding towards 0
#define FIXED_TO_INT_TRUNC(x) ((x) / FACTOR)

// fixed point number x to integer rounding to the nearest int
#define FIXED_TO_INT(x) (((x) >= 0) ? ((x) + FACTOR / 2) / FACTOR : ((x) - FACTOR / 2) / FACTOR)

#define FIXED_ADD(x, y) ((x) + (y))

#define FIXED_ADD_INT(x, n) ((x) + ((n) * FACTOR))

#define FIXED_SUB(x, y) ((x) - (y))

#define FIXED_SUB_INT(x, n) ((x) - ((n) * FACTOR))

#define FIXED_MUL(x, y) (((int64_t) x) * (y) / FACTOR)

#define FIXED_MUL_INT(x, n) ((x) * (n))

#define FIXED_DIV(x, y) (((int64_t) x) * FACTOR / (y))

#define FIXED_DIV_INT(x, n) ((x) / (n))

#endif /* threads/fixed-point.h */