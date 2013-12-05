#include "globals.h"

Board* g_boardx[2]; 
s32bit g_board_size[2] = {-1,-1};

extern void
initialize_board(s32bit num_rows, s32bit num_cols, s32bit board[30][30])
{
  // Check if we need to re-initialize the solver.
  //bool init = 1; //(g_trans_table == NULL || !horz || !vert ||
              // horz->num_rows != num_rows || vert->num_rows != num_cols);

  g_board_size[HORIZONTAL] = num_rows;
  g_board_size[VERTICAL]   = num_cols;

  Board* horz = g_boardx[HORIZONTAL] = new Board(num_rows, num_cols);
  g_boardx[VERTICAL] = horz->GetOpponent();

  init_hashtable(num_rows, num_cols, board);
  
  horz->Print();
  printf("\n");
  horz->PrintInfo();
}
