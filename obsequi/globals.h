#ifndef GLOBALS_H
#define GLOBALS_H

#include "utils.h"

#include "consts.h"
#include "board.h"
#include "hash-table.h"

// Global variables.
extern Board* g_boardx[2];
extern s32bit g_board_size[2];
extern s32bit g_empty_squares;

//########################################################
// This function gives the solver the current board that
//   we want to work with.
//########################################################
void   initialize_board  (s32bit row, s32bit col,
                          s32bit board[30][30]);


//########################################################
// This function tries to find the best move for 'player'.
//
//  - player is either 'V' or 'H'.
//########################################################
s32bit search_for_move   (char player,
                          s32bit *row, s32bit *col, u64bit *nodes);


//########################################################
// This function returns a string about the current
//   state of the search.
//########################################################
void current_search_state();


#endif //ifndef GLOBALS_H
