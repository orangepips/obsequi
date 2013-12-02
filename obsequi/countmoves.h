// Return the number of non-overlapping moves in a bitmap

#ifndef COUNTMOVES_H
#define COUNTMOVES_H

#include "utils.h"

#define ERASE_NEXT_BIT 0x800000

extern s32bit move_table16[65536];

static inline int
countmoves32_calc(u32bit mask) {
  s32bit count = 0;
  u32bit tmp;
  
  while(mask){
    tmp = (mask&-mask);           // least sig bit of m
    mask &= ~(tmp | (tmp << 1));  // remove bit and next bit.
    count++;
  }
  return count;
}

static inline int
countmoves16_16bitmap(u32bit mask) {
  // Strip off any of those special flags...
  return move_table16[mask] & 0xFF;
}

static inline int
countmoves32_16bitmap(u32bit mask)
{
  s32bit count = 0;
  count = move_table16[mask & 0xFFFF];

  // Special flag was set, remove first bit from next half.
  if (count & ERASE_NEXT_BIT) {
    count += (move_table16[(mask >> 16) & 0xFFFE]);
  } else {
    count += (move_table16[mask >> 16]);
  }

  // Strip off any of those special flags...
  return count & 0xFF;
}

// Determine which version we are going to use.
// You can run the tests in countmoves.c to determine which is fastest.
static inline int
countmoves32(u32bit x) {
#ifdef BOARD_SIZE_LT_16
  return countmoves16_16bitmap(x);  // About twice as fast as 32 bit version.
#else
  return countmoves32_16bitmap(x);
#endif
}

#endif // COUNTMOVES_H
