// Return the index of the last bit set (bfs)

#ifndef LASTBIT_H
#define LASTBIT_H

#include "utils.h"

extern s32bit lastbit8[256];
extern s32bit lastbit16[65536];

static inline int
lastbit32_8bitmap(u32bit x) {
  if (x & 0x000000FF)
    return (lastbit8[x & 0x000000FF]);
  if (x & 0x0000FF00)
    return (lastbit8[(x>>8) & 0x000000FF] + 8);
  if (x & 0x00FF0000)
    return (lastbit8[(x>>16) & 0x000000FF] + 16);
  if (x & 0xFF000000)
    return (lastbit8[(x>>24) & 0x000000FF] + 24);
  return 100;
}

static inline int
lastbit32_16bitmap(u32bit x) {
  /* returns the position of the last bit in x */
  if (x & 0x0000FFFF)
    return (lastbit16[x & 0x0000FFFF]);
  if (x & 0xFFFF0000)
    return (lastbit16[(x>>16) & 0x0000FFFF] + 16);
  return 100;
}

static inline int
lastbit32_gccbuiltin(u32bit x) {
  if (!x) return 100;
  return __builtin_ffs(x)-1;
}

// Determine which version we are going to use.
// You can run the tests in lastbit.c to determine which is fastest.
static inline int
lastbit32(u32bit x) {
  return lastbit32_16bitmap(x);
}

#endif // LASTBIT_H
