// Count the number of bits set

#ifndef COUNTBITS_H
#define COUNTBITS_H

#include "utils.h"

extern s32bit countbits8[256];
extern s32bit countbits16[65536];

static inline int
countbits32_8bitmap(u32bit x) {
  return (countbits8[x & 0x000000FF] +
          countbits8[(x >> 8) & 0x000000FF] +
          countbits8[(x >> 16) & 0x000000FF] +
          countbits8[(x >> 24) & 0x000000FF]);
}

static inline int
countbits32_16bitmap(u32bit x) {
  return (countbits16[x & 0x0000FFFF] +
          countbits16[(x >> 16) & 0x0000FFFF]);
}

static inline int
countbits32_gccbuiltin(u32bit x) {
  return __builtin_popcount(x);
}

// Determine which version we are going to use.
// You can run the tests in countbits.h to determine which is fastest.
static inline int
countbits32(u32bit x) {
  return countbits32_gccbuiltin(x);
}

#endif // COUNTBITS_H
