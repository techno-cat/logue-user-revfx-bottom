/*
Copyright 2019 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "userrevfx.h"
#include "buffer_ops.h"
#include "LCWCommon.h"
#include "LCWFilterIir.h"
#include "LCWDelayBuffer.h"
#include "LCWReverbParam.h"

static __sdram int32_t s_reverb_ram_pre_buffer[LCW_REVERB_PRE_SIZE];
static __sdram int32_t s_reverb_ram_comb_buffer[LCW_REVERB_COMB_BUFFER_TOTAL];
static __sdram int32_t s_reverb_ram_ap_buffer[LCW_REVERB_AP_BUFFER_TOTAL];

static LCWDelayBuffer revPreBuffer;
static LCWDelayBuffer revCombBuffers[LCW_REVERB_COMB_MAX];
static LCWAPFBuffer revApBuffers[LCW_REVERB_AP_MAX];

static float s_inputGain;
static float s_mix;
static float s_depth;
static float s_time;
static LCWFilterIir2 s_combIir2[LCW_REVERB_COMB_MAX];

__fast_inline float softlimiter(float c, float x)
{
  float xf = si_fabsf(x);
  if ( xf < c ) {
    return x;
  }
  else {
    return si_copysignf( c + fx_softclipf(c, xf - c), x );
  }
}

void REVFX_INIT(uint32_t platform, uint32_t api)
{
  for (int32_t i=0; i<LCW_REVERB_COMB_MAX; i++) {
    LCWDelayBuffer *buf = &(revCombBuffers[i]);
    buf->buffer = &(s_reverb_ram_comb_buffer[LCW_REVERB_COMB_SIZE * i]);
    buf->size = LCW_REVERB_COMB_SIZE;
    buf->mask = LCW_REVERB_COMB_SIZE - 1;
    buf->pointer = 0;
    buf->fbGain = LCW_SQ15_16( 0.7 ) >> 1; // q15
    buf->offset = lcwCombDelaySize[i];
    buf->out = 0;

/*
=== LPF ( Cutoff: 0.080, Q: 0.707 ) ===
b0:  0.046132 (0x000BCF4B)
b1:  0.092264 (0x00179E96)
b2:  0.046132 (0x000BCF4B)
a1: -1.307285 (0xFEB155C5)
a2:  0.491812 (0x007DE768)
*/
    LCWFilterIir2 *iir = &s_combIir2[0] + i;
    iir->b0 = 0x000BCF4B;
    iir->b1 = 0x00179E96;
    iir->b2 = 0x000BCF4B;
    iir->a1 = 0xFEB155C5;
    iir->a2 = 0x007DE768;
    iir->z1 = 0;
    iir->z2 = 0;
  }

// (953) // = 48000 * 0.020
// (331) // = 48000 * 0.0068
// (71)  // = 48000 * 0.0015
// (241) // = 48000 * 0.005
// (81)  // = 48000 * 0.0017
// (23)  // = 48000 * 0.0005
  const int32_t apDelay[] = {
    953, 241,
    331, 81,
    71, 23
  };

  for (int32_t i=0; i<LCW_REVERB_AP_MAX; i++) {
    LCWAPFBuffer *buf = &(revApBuffers[i]);
    buf->buffer = &(s_reverb_ram_ap_buffer[LCW_REVERB_AP_SIZE * i]);
    buf->size = LCW_REVERB_AP_SIZE;
    buf->mask = LCW_REVERB_AP_SIZE - 1;
    buf->pointer = 0;
    buf->gain = LCW_SQ15_16( 0.7 );
    buf->offset = apDelay[i];
    buf->out = 0;
  }

  {
    LCWDelayBuffer *buf = &(revPreBuffer);
    buf->buffer = &(s_reverb_ram_pre_buffer[LCW_REVERB_PRE_SIZE]);
    buf->size = LCW_REVERB_PRE_SIZE;
    buf->mask = LCW_REVERB_PRE_SIZE - 1;
    buf->pointer = 0;
    buf->fbGain = 0;
    buf->offset = 0;
    buf->out = 0;
  }

  s_mix = 0.5f;
  s_depth = 0.f;
  s_time = 0.f;
  s_inputGain = 0.f;
}

#define FIR_TAP_PREPARE (9)
__fast_inline int32_t prepareProcess(LCWDelayBuffer *p, int32_t inL)
{
  p->pointer = LCW_DELAY_BUFFER_DEC(p);
  p->buffer[p->pointer] = inL;

// fc = 0.14, tap = 9 (orange)
  const int32_t i = p->pointer;
  const int32_t mask = p->mask;
  const int32_t fir[FIR_TAP_PREPARE] = {
    -2457, 11509, 88522, 222526, -754975, 222526, 88522, 11509, -2457
  };

  int64_t sum = 0;
  for (int32_t j=0; j<FIR_TAP_PREPARE; j++) {
    const int64_t tmp = p->buffer[(i + j) & mask];
    sum += (tmp * (int64_t)fir[j]);
  }

  return (int32_t)(sum >> 20);
}

void inputCombLines(int32_t *outL, int32_t in, LCWDelayBuffer *comb)
{
  int32_t out[LCW_REVERB_COMB_MAX];
  for (int32_t j=0; j<LCW_REVERB_COMB_MAX; j++) {
    LCWDelayBuffer *p = comb + j;
    const int32_t z = iir2_input(
      &s_combIir2[j], LCW_DELAY_BUFFER_LUT(p, p->offset) );

    // 軽量ver.
    int32_t tmp = 0;
    if ( z != -1 ) {
      tmp = (int32_t)( ((int64_t)z * p->fbGain) >> 15 );
    }

    p->pointer = LCW_DELAY_BUFFER_DEC(p);
    p->buffer[p->pointer] = in + tmp;

    out[j] = z;
  }

  // memo:
  // ここで符号を変えて計算したものを2つ用意すればステレオ対応できる（？）
  *outL = out[0] - out[1] + out[2] - out[3];
}

__fast_inline int32_t inputAllPass(int32_t in, LCWAPFBuffer *p)
{
  const int32_t z_out = LCW_DELAY_BUFFER_LUT(p, p->offset);
  const int64_t in2 = in - (((int64_t)z_out * p->gain) >> 16);

  const int32_t out = z_out + (int32_t)((in2 * p->gain) >> 16);

  p->pointer = LCW_DELAY_BUFFER_DEC(p);
  p->buffer[p->pointer] = in2;

  return out;
}

__fast_inline int32_t inputAllPass2(int32_t in, LCWAPFBuffer *buffer)
{
  int32_t out = in;

  for (int32_t j=0; j<LCW_REVERB_AP_MAX; j+=2) {
    LCWAPFBuffer *p = buffer + j;

    // 内側の処理
    const int32_t innerOut = inputAllPass(
      LCW_DELAY_BUFFER_LUT(p, p->offset), p + 1 );

    // 外側の処理
    {
      const int64_t in = out;
      const int32_t z_out = innerOut;
      const int64_t in2 = in - (((int64_t)z_out * p->gain) >> 16);

      out = z_out + (int32_t)((in2 * p->gain) >> 16);

      p->pointer = LCW_DELAY_BUFFER_DEC(p);
      p->buffer[p->pointer] = in2;
    }
  }

  return out;
}

void REVFX_PROCESS(float *xn, uint32_t frames)
{
  float * __restrict x = xn;
  const float * x_e = x + 2*frames;

  const float dry = 1.f - s_mix;
  const float wet = s_mix;
  const int32_t depth = (int32_t)(s_depth * 0x100);

  const int32_t time = (int32_t)((LCW_REVERB_GAIN_TABLE_SIZE - 1) * s_time);

  for (int32_t i=0; i<LCW_REVERB_COMB_MAX; i++) {
    revCombBuffers[i].fbGain = lcwReverbGainTable[time][i];
  }

  LCWDelayBuffer *pre = &revPreBuffer;
  LCWDelayBuffer *comb = &(revCombBuffers[0]);
  LCWAPFBuffer *ap = &revApBuffers[0];

  int32_t combSrc[LCW_REVERB_COMB_MAX];
  for (; x != x_e; ) {
    float xL = *x;
    // float xR = *(x + 1);
    const int32_t inL = (int32_t)( s_inputGain * xL * (1 << 20) );

    // prepare
    const int32_t preOut = prepareProcess( pre, inL );

    // comb-filter
    int32_t combSum;
    inputCombLines( &combSum, preOut, comb);

    // all-pass
    const int32_t apOut = inputAllPass2( combSum >> 2, ap );

    float outL = apOut / (float)(1 << 20);
    float yL = softlimiter( 0.1f, (dry * xL) + (wet * outL) );

    *(x++) = yL;
    *(x++) = yL;

    if ( s_inputGain < 0.99998f ) {
      s_inputGain += ( (1.f - s_inputGain) * 0.0625f );
    }
    else { s_inputGain = 1.f; }
  }
}

void REVFX_RESUME(void)
{
  buf_clr_u32(
    (uint32_t * __restrict__)s_reverb_ram_pre_buffer,
    LCW_REVERB_PRE_SIZE );
  buf_clr_u32(
    (uint32_t * __restrict__)s_reverb_ram_comb_buffer,
    LCW_REVERB_COMB_BUFFER_TOTAL );
  buf_clr_u32(
    (uint32_t * __restrict__)s_reverb_ram_ap_buffer,
    LCW_REVERB_AP_BUFFER_TOTAL );
  s_inputGain = 0.f;
}

void REVFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case k_user_revfx_param_time:
    s_time = clip01f(valf);
    break;
  case k_user_revfx_param_depth:
    s_depth = clip01f(valf);
    break;
  case k_user_revfx_param_shift_depth:
    // Rescale to add notch around 0.5f
    s_mix = (valf <= 0.49f) ? 1.02040816326530612244f * valf : (valf >= 0.51f) ? 0.5f + 1.02f * (valf-0.51f) : 0.5f;
    break;
  default:
    break;
  }
}
