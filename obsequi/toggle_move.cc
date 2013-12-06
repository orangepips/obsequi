
#include "globals.h"
#include "macros.h"


//#define DEBUG_SCORE_MOVE

static inline s32bit
score_move(Board* boardx, Move move, s32bit player)
{
  u32bit* board = boardx->board;
  u32bit* opp_board = boardx->GetOpponent()->board;
    
  int row = move.array_index;
  int col = move.mask_index;
  
  board[row]   ^= (3<<col);
  opp_board[col]   ^= (1<<row);
  opp_board[col+1] ^= (1<<row);
  
  Board* horz = boardx;
  Board* vert = boardx->GetOpponent();

  // update real moves
  int score = count_real(board, row) - horz->info[row].real;

  score -= count_real(opp_board, col) - vert->info[col].real;
  score -= count_real(opp_board, col + 1) - vert->info[col+1].real;

  // update safe moves
  if(row - 1 != 0)
    score += count_safe(board, row - 1) - horz->info[row-1].safe;
  score += count_safe(board, row) - horz->info[row].safe;
  if(row != horz->GetNumRows())
    score += count_safe(board, row+1) - horz->info[row+1].safe;
  
  if(col - 1 != 0)
    score -= count_safe(opp_board, col - 1) - vert->info[col-1].safe;
  if(col + 1 != vert->GetNumRows())
    score -= count_safe(opp_board, col + 2) - vert->info[col+2].safe;
  
  board[row]   ^= (3<<col);
  opp_board[col]   ^= (1<<row);
  opp_board[col+1] ^= (1<<row);
  
  score *= 128;
  score += g_first_move[player][row][col];

  return score;
}

extern void
score_and_get_first(Board* board, Move movelist[MAXMOVES], s32bit num_moves,
                    s32bit player, Move first)
{
  s32bit i, max = -50000, max_index = -1;

  //========================================================
  // Give the move from the hashtable a large sorting value.
  //========================================================
  /*
  {
    int z = 0;
    for (z = 0; z < num_moves; z++)
      printf("> %d %d\n", movelist[z].array_index, movelist[z].mask_index);
  }
  */
  if(first.array_index != -1){
    for(i = 0; i < num_moves; i++){
      if(first.array_index == movelist[i].array_index
         && first.mask_index == movelist[i].mask_index){
        movelist[i].info = 450000;
        max_index = i;
      } else {
        movelist[i].info = score_move(board, movelist[i], player);
      }
    }
  }

  else {
    for(i = 0; i < num_moves; i++){
      movelist[i].info = score_move(board, movelist[i], player);
      if(movelist[i].info > max){
        max = movelist[i].info;
        max_index = i;
      }
    }
  }
  
  if(max_index == -1) fatal_error(1, "No maximum\n");
  
  // put biggest at front
  if(num_moves > 1) {
    Move tmp_move = movelist[max_index];
    for(i = max_index; i > 0; i--)
      movelist[i] = movelist[i - 1];
    movelist[0] = tmp_move;
  }
}
