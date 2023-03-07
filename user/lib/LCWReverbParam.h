/*
Copyright 2023 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include <stdint.h>
#include "LCWCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// 一時バッファと初期反射バッファ
#define LCW_REVERB_PRE_SIZE (1<<12)
#define LCW_REVERB_PRE_MAX (2)
#define LCW_REVERB_PRE_BUFFER_TOTAL (LCW_REVERB_PRE_SIZE * LCW_REVERB_PRE_MAX)

#define LCW_REVERB_COMB_SIZE (1<<12)
#define LCW_REVERB_COMB_MAX (4)
#define LCW_REVERB_COMB_BUFFER_TOTAL (LCW_REVERB_COMB_SIZE * LCW_REVERB_COMB_MAX)

#define LCW_REVERB_AP_SIZE (1<<12)
#define LCW_REVERB_AP_MAX (6)
#define LCW_REVERB_AP_BUFFER_TOTAL (LCW_REVERB_AP_SIZE * LCW_REVERB_AP_MAX)

#define LCW_REVERB_GAIN_TABLE_SIZE (48 + 1)

extern const int32_t lcwCombDelaySize[LCW_REVERB_COMB_MAX];

// q15
extern const uint16_t lcwReverbGainTable[LCW_REVERB_GAIN_TABLE_SIZE][LCW_REVERB_COMB_MAX];

#ifdef __cplusplus
}
#endif
