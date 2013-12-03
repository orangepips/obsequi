// Functions for generating the moves that can be made in a given
// board position.
#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "consts.h"
#include "structs.h"
#include "utils.h"

// Generate all the moves that can be made.
// Places x moves into movelist and returns x.
int move_generator(int rows, u32bit board[MAX_ROWS],
                   Move movelist[MAX_MOVES]);

// Generate moves in two parts. (Union of these two stages should equal above.)
// - stage1 is all the .... moves.
// - stage2 is all the .... moves.
// Places x moves into movelist and returns x.
int move_generator_stage1(int rows, u32bit board[MAX_ROWS],
                          Move movelist[MAX_MOVES]);
int move_generator_stage2(int rows, u32bit board[MAX_ROWS],
                          int start, Move movelist[MAX_MOVES]);

#endif //ifndef MOVE_GEN_H
