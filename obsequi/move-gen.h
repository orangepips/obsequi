// Functions for generating the moves that can be made in a given
// board position.
#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "consts.h"
#include "structs.h"
#include "utils.h"

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

#endif //ifndef MOVE_GEN_H
