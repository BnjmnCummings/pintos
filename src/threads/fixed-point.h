#ifndef THREADS_FIXED_POINT_H
#define THREADS_FIXED_POINT_H

//not sure if this will have to change
#define N_FRACTIONAL_BITS 14
#define SCALE_FACTOR (2^N_FRACTIONAL_BITS)

#define INT_TO_FXP(n) (n * SCALE_FACTOR)

// fixed point number x to integer rounding towards 0
#define FXP_TO_INT0(x) (x / SCALE_FACTOR)

// fixed point number x to integer rounding to the nearest int
#define FXP_TO_INT(x) //todo

#endif /* threads/fixed-point.h */