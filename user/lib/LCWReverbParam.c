/*
Copyright 2023 Tomoaki Itoh
This software is released under the MIT License, see LICENSE.txt.
//*/

#include "LCWReverbParam.h"

const int32_t lcwCombDelaySize[LCW_REVERB_COMB_MAX] = {
  1693, 1759, 1823, 1907
};

// q15
const uint16_t lcwReverbGainTable[LCW_REVERB_GAIN_TABLE_SIZE][LCW_REVERB_COMB_MAX] = {
  { 0, 0, 0, 0 }, // [ 0]
//1693, 1759, 1823, 1907
  { 0x7261, 0x71E1, 0x7165, 0x70C3 }, // [ 1] 2.165
  { 0x735D, 0x72E6, 0x7273, 0x71DC }, // [ 2] 2.344
  { 0x7449, 0x73DA, 0x736E, 0x72E2 }, // [ 3] 2.538
  { 0x7524, 0x74BC, 0x7458, 0x73D5 }, // [ 4] 2.748
  { 0x75EF, 0x758F, 0x7532, 0x74B8 }, // [ 5] 2.975
  { 0x76AD, 0x7653, 0x75FD, 0x758C }, // [ 6] 3.221
  { 0x775D, 0x770A, 0x76B9, 0x7650 }, // [ 7] 3.487
  { 0x7800, 0x77B3, 0x7768, 0x7707 }, // [ 8] 3.775
  { 0x7898, 0x7850, 0x780B, 0x77B0 }, // [ 9] 4.088
  { 0x7925, 0x78E2, 0x78A2, 0x784E }, // [10] 4.425
  { 0x79A7, 0x796A, 0x792E, 0x78E0 }, // [11] 4.791
  { 0x7A21, 0x79E7, 0x79B0, 0x7967 }, // [12] 5.187
  { 0x7A91, 0x7A5C, 0x7A28, 0x79E5 }, // [13] 5.616
  { 0x7AF9, 0x7AC8, 0x7A98, 0x7A5A }, // [14] 6.080
  { 0x7B59, 0x7B2C, 0x7B00, 0x7AC6 }, // [15] 6.583
  { 0x7BB3, 0x7B89, 0x7B60, 0x7B2A }, // [16] 7.127
  { 0x7C06, 0x7BDE, 0x7BB9, 0x7B87 }, // [17] 7.716
  { 0x7C52, 0x7C2E, 0x7C0B, 0x7BDD }, // [18] 8.354
  { 0x7C99, 0x7C78, 0x7C57, 0x7C2D }, // [19] 9.045
  { 0x7CDB, 0x7CBC, 0x7C9E, 0x7C76 }, // [20] 9.792
  { 0x7D18, 0x7CFB, 0x7CDF, 0x7CBB }, // [21] 10.602
  { 0x7D50, 0x7D35, 0x7D1C, 0x7CFA }, // [22] 11.478
  { 0x7D84, 0x7D6B, 0x7D53, 0x7D34 }, // [23] 12.427
  { 0x7DB4, 0x7D9D, 0x7D87, 0x7D6A }, // [24] 13.454
  { 0x7DE0, 0x7DCB, 0x7DB7, 0x7D9C }, // [25] 14.567
  { 0x7E0A, 0x7DF6, 0x7DE3, 0x7DCB }, // [26] 15.771
  { 0x7E30, 0x7E1E, 0x7E0C, 0x7DF6 }, // [27] 17.074
  { 0x7E53, 0x7E42, 0x7E32, 0x7E1D }, // [28] 18.486
  { 0x7E74, 0x7E64, 0x7E55, 0x7E42 }, // [29] 20.014
  { 0x7E92, 0x7E83, 0x7E76, 0x7E64 }, // [30] 21.668
  { 0x7EAD, 0x7EA0, 0x7E94, 0x7E83 }, // [31] 23.459
  { 0x7EC7, 0x7EBB, 0x7EAF, 0x7EA0 }, // [32] 25.398
  { 0x7EDF, 0x7ED4, 0x7EC9, 0x7EBB }, // [33] 27.498
  { 0x7EF5, 0x7EEB, 0x7EE1, 0x7ED3 }, // [34] 29.771
  { 0x7F09, 0x7F00, 0x7EF6, 0x7EEA }, // [35] 32.232
  { 0x7F1C, 0x7F13, 0x7F0B, 0x7EFF }, // [36] 34.896
  { 0x7F2D, 0x7F25, 0x7F1D, 0x7F13 }, // [37] 37.781
  { 0x7F3D, 0x7F36, 0x7F2F, 0x7F25 }, // [38] 40.904
  { 0x7F4C, 0x7F45, 0x7F3E, 0x7F36 }, // [39] 44.285
  { 0x7F5A, 0x7F53, 0x7F4D, 0x7F45 }, // [40] 47.946
  { 0x7F67, 0x7F61, 0x7F5B, 0x7F53 }, // [41] 51.909
  { 0x7F72, 0x7F6D, 0x7F67, 0x7F60 }, // [42] 56.200
  { 0x7F7D, 0x7F78, 0x7F73, 0x7F6D }, // [43] 60.846
  { 0x7F87, 0x7F82, 0x7F7E, 0x7F78 }, // [44] 65.875
  { 0x7F90, 0x7F8C, 0x7F88, 0x7F82 }, // [45] 71.321
  { 0x7F99, 0x7F95, 0x7F91, 0x7F8C }, // [46] 77.216
  { 0x7FA1, 0x7F9D, 0x7F99, 0x7F95 }, // [47] 83.599
  { 0x7FA8, 0x7FA4, 0x7FA1, 0x7F9D }, // [48] 90.510
};


