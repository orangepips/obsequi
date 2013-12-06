// Functions for generating the moves that can be made in a given
// board position.
#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "consts.h"
#include "utils.h"
#include "board.h"

//########################################################
// Info we need to describe a move.
//########################################################
class Move {
 public:
  s32bit array_index;
  s32bit mask_index;
  s32bit info;
};

// Generate all the moves that can be made.
// Places x moves into movelist and returns x.
int move_generator(const Board& board,
                   Move movelist[MAX_MOVES]);

// Generate moves in two parts. (Union of these two stages should equal above.)
// - stage1 is all the .... moves.
// - stage2 is all the .... moves.
// Places x moves into movelist and returns x.
int move_generator_stage1(const Board& board,
                          Move movelist[MAX_MOVES]);
int move_generator_stage2(const Board& board,
                          int start, Move movelist[MAX_MOVES]);

// Score all the moves (and move the best to the front of the list).
void score_and_get_first(Board* board, Move movelist[MAXMOVES], 
    s32bit num_moves, Move first);

// Sort moves into a decending order. (Stable sort.)
void sort_moves(Move movelist[MAXMOVES], s32bit start, s32bit num_moves);


#endif //ifndef MOVE_GEN_H
