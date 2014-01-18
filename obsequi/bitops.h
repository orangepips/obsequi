#ifndef OBSEQUI_BITOPS_H
#define OBSEQUI_BITOPS_H

#include "base.h"

namespace obsequi {

namespace internal {
  extern int lastbit8[256];
  extern int lastbit16[65536];

  static inline int lastbit32_8bitmap(u32bit x) {
    if ((x>>0)  & 0xFF) return (lastbit8[(x>>0)  & 0xFF]);
    if ((x>>8)  & 0xFF) return (lastbit8[(x>>8)  & 0xFF] + 8);
    if ((x>>16) & 0xFF) return (lastbit8[(x>>16) & 0xFF] + 16);
    if ((x>>24) & 0xFF) return (lastbit8[(x>>24) & 0xFF] + 24);
    return 100;
  }

  static inline int lastbit32_16bitmap(u32bit x) {
    if ((x>>0)  & 0xFFFF) return (lastbit16[(x>>0)  & 0xFFFF]);
    if ((x>>16) & 0xFFFF) return (lastbit16[(x>>16) & 0xFFFF] + 16);
    return 100;
  }

  static inline int lastbit32_gccbuiltin(u32bit x) {
    if (!x) return 100;
    return __builtin_ffs(x)-1;
  }
}  // namespace obsequi::internal

// Return index of lowest order bit set. 0x1 returns 0, 0x2 return 1, etc.
// Return value for 0x0 is undefined.
static inline int lastbit32(u32bit x) {
  // You can run the tests to determine which is fastest.
  return internal::lastbit32_16bitmap(x);
}


namespace internal {
  extern int countbits8[256];
  extern int countbits16[65536];

  static inline int countbits32_8bitmap(u32bit x) {
    return (countbits8[x & 0xFF] +
            countbits8[(x>>8) & 0xFF] +
            countbits8[(x>>16) & 0xFF] +
            countbits8[(x>>24) & 0xFF]);
  }

  static inline int countbits32_16bitmap(u32bit x) {
    return (countbits16[x & 0xFFFF] + countbits16[(x >> 16) & 0xFFFF]);
  }

  static inline int countbits32_gccbuiltin(u32bit x) {
    return __builtin_popcount(x);
  }
}  // namespace obsequi::internal

// Return count of total number of bits in x.
static inline int countbits32(u32bit x) {
  // You can run the tests to determine which is fastest.
  return internal::countbits32_gccbuiltin(x);
}


namespace internal {
  const u32bit ERASE_NEXT_BIT = 0x800000;
  extern int move_table16[65536];

  static inline int countmoves16_16bitmap(u32bit mask) {
    // Strip off any of those special flags...
    return move_table16[mask] & 0xFF;
  }

  static inline int countmoves32_16bitmap(u32bit mask) {
    int count = move_table16[mask & 0xFFFF];

    // Special flag was set, remove first bit from next half.
    if (count & ERASE_NEXT_BIT) {
      count += (move_table16[(mask >> 16) & 0xFFFE]);
    } else {
      count += (move_table16[mask >> 16]);
    }

    // Strip off any of those special flags...
    return count & 0xFF;
  }
}  // namespace obsequi::internal

// Return the number of non-overlapping moves in a bitmap
static inline int countmoves32(u32bit x) {
  return internal::countmoves32_16bitmap(x);
}

}  // namespace obsequi
#endif  // OBSEQUI_BITOPS_H
