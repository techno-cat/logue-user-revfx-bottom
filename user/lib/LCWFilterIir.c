/*
Copyright 2022 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "LCWFilterIir.h"

int32_t iir1_input(LCWFilterIir1 *iir, int32_t in)
{
  const int64_t z1 = iir->z1;

  const int64_t in2 = ( (((int64_t)in) << LCW_FILTER_FRAC_BITS) - (iir->a1 * z1) ) >> LCW_FILTER_FRAC_BITS;
  const int64_t out = ( (in2 * iir->b0) + (z1 * iir->b1) ) >> LCW_FILTER_FRAC_BITS;

  iir->z1 = in2;

  return (int32_t)out;
}

// b0とb1が同じ値を想定
int32_t iir1_input_opt(LCWFilterIir1 *iir, int32_t in)
{
  const int64_t z1 = iir->z1;

  const int64_t in2 = ( (((int64_t)in) << LCW_FILTER_FRAC_BITS) - (iir->a1 * z1) ) >> LCW_FILTER_FRAC_BITS;
  // const int64_t out = ( (in2 * iir->b0) + (z1 * iir->b1) ) >> LCW_FILTER_FRAC_BITS;
  const int64_t out = ( (in2 + z1) * iir->b0  ) >> LCW_FILTER_FRAC_BITS;

  iir->z1 = in2;

  return (int32_t)out;
}

int32_t iir2_input(LCWFilterIir2 *iir, int32_t in)
{
  const int64_t z1 = iir->z1;
  const int64_t z2 = iir->z2;

  const int64_t in2 = ( (((int64_t)in) << LCW_FILTER_FRAC_BITS) - (iir->a1 * z1) - (iir->a2 * z2) ) >> LCW_FILTER_FRAC_BITS;
  const int64_t out = ( (iir->b0 * in2) + (iir->b1 * z1) + (iir->b2 * z2) ) >> LCW_FILTER_FRAC_BITS;

  iir->z2 = z1;
  iir->z1 = in2;

  return (int32_t)out;
}

// Q = 1/sqrt(2)を想定
int32_t iir2_input_opt(LCWFilterIir2 *iir, int32_t in)
{
  const int64_t z1 = iir->z1;
  const int64_t z2 = iir->z2;

  const int64_t in2 = ( (((int64_t)in) << LCW_FILTER_FRAC_BITS) - (iir->a1 * z1) - (iir->a2 * z2) ) >> LCW_FILTER_FRAC_BITS;
  // const int64_t out = ( (iir->b0 * in2) + (iir->b1 * z1) + (iir->b2 * z2) ) >> LCW_FILTER_FRAC_BITS;
  const int64_t out = ( (iir->b0 * (in2 + z2)) + (iir->b1 * z1) ) >> LCW_FILTER_FRAC_BITS;

  iir->z2 = z1;
  iir->z1 = in2;

  return (int32_t)out;
}
