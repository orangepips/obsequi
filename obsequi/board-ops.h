// Class to handle the board state.
#ifndef BOARD_OPS_H
#define BOARD_OPS_H

#include "countmoves.h"

//########################################################
// This function counts the number of safe moves in a row.
//
// Never call this function for rows 0 or 31.
//########################################################
static inline int
count_safe(const u32bit board[32], s32bit row)
{
  u32bit guard = board[row-1] & board[row+1];

  // mask contains a bit for each safe move.
  u32bit mask= ( (~(board[row] | (board[row] << 1)))
                 & (guard & (guard << 1)) );

  return countmoves32(mask);
}

//########################################################
// This function counts the number of real moves in a row.
//
// Never call this function for rows 0 or 31.
//########################################################
static inline int
count_real(const u32bit board[32], s32bit row)
{
  // mask contains a bit for each real move.
  u32bit mask= ~(board[row] | (board[row] << 1));

  return countmoves32(mask);
}

#endif  // BOARD_H
