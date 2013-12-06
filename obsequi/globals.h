#ifndef GLOBALS_H
#define GLOBALS_H

#include "utils.h"

#include "structs.h"
#include "consts.h"
#include "board.h"
#include "hash-table.h"

//========================================================
// Give score of move relative to current position.
//========================================================
//s32bit  score_move(Move move, s32bit player);
void
score_and_get_first(Board* board, Move movelist[MAXMOVES], s32bit num_moves,
                    Move first);

//========================================================
// Use the value of movelist[i].info to sort the moves in
//   descending order.
//========================================================
void    sort_moves(Move movelist[MAXMOVES], s32bit start, s32bit num_moves);

//########################################################
// Functions for information display or sanity checks.
//========================================================
void    check_board();
void    check_board_info();

//========================================================
// Functions which print various pieces of information in a
//   readable format.
//========================================================
void    print_u64bit(u64bit val);

//########################################################
// Global variables.
//########################################################
extern u64bit g_num_nodes;
extern Board* g_boardx[2];
extern s32bit g_board_size[2];
extern s32bit g_empty_squares;
extern s32bit g_move_number[128];

#endif //ifndef GLOBALS_H
