/*
Copyright 2022 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LCW_FILTER_FRAC_BITS (24)

typedef struct {
  int64_t z1;
  int32_t a1;
  int32_t b0, b1;
} LCWFilterIir1;

typedef struct {
  int64_t z1, z2;
  int32_t a1, a2;
  int32_t b0, b1, b2;
} LCWFilterIir2;

extern int32_t iir1_input(LCWFilterIir1 *iir, int32_t in);

// b0とb1が同じ値を想定
extern int32_t iir1_input_opt(LCWFilterIir1 *iir, int32_t in);

extern int32_t iir2_input(LCWFilterIir2 *iir, int32_t in);

// Q = 1/sqrt(2)を想定
extern int32_t iir2_input_opt(LCWFilterIir2 *iir, int32_t in);

#ifdef __cplusplus
}
#endif
