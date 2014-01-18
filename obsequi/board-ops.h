#ifndef OBSEQUI_BOARD_OPS_H
#define OBSEQUI_BOARD_OPS_H

#include "base.h"
#include "bitops.h"

namespace obsequi {

// Count non-overlapping safe moves in a row.
// NOTE: Never call this function for rows 0 or 31.
static inline int count_safe(const u32bit board[32], s32bit row) {
  u32bit guard = board[row-1] & board[row+1];

  // mask contains a bit for each safe move.
  u32bit mask= ( (~(board[row] | (board[row] << 1)))
                 & (guard & (guard << 1)) );

  return countmoves32(mask);
}

// Count non-overlapping moves in a row.
static inline int count_real(const u32bit board[32], s32bit row) {
  u32bit mask= ~(board[row] | (board[row] << 1));

  return countmoves32(mask);
}

// Count total number of moves in a row.
static inline int count_total(const u32bit board[32], s32bit row) {
  u32bit mask= ~(board[row] | (board[row] << 1));

  return countbits32(mask);
}

}  // namespace obsequi
#endif  // OBSEQUI_BOARD_OPS_H
