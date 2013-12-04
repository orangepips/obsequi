
#include "globals.h"
#include "macros.h"

//########################################################
// This function counts the number of safe moves in a row.
//
// Never call this function for rows 0 or 31.
//########################################################
static inline int
count_safe(int num_rows, u32bit board[32], s32bit row)
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
count_real(int num_rows, u32bit board[32], s32bit row)
{
  // mask contains a bit for each real move.
  u32bit mask= ~(board[row] | (board[row] << 1));
  
  return countmoves32(mask);
}


static inline void
_update_safe(int num_rows, u32bit board[32], s32bit player, s32bit row)
{
  s32bit count = count_safe(num_rows, board, row);

  g_info_totals[player].safe += count - g_info[player][row].safe;
  g_info[player][row].safe    = count;
}

static inline void
_update_real(int num_rows, u32bit board[32], s32bit player, s32bit row)
{
  s32bit count = count_real(num_rows, board, row);
  
  g_info_totals[player].real += count - g_info[player][row].real;
  g_info[player][row].real = count;
}



extern void
update_safe(int num_rows, u32bit board[32], s32bit player, s32bit row)
{ _update_safe(num_rows, board, player, row); }

extern void
update_real(int num_rows, u32bit board[32], s32bit player, s32bit row)
{ _update_real(num_rows, board, player, row); }

extern void
toggle_move(Move move, s32bit player)
{
  s32bit row, col, horz_play, vert_play;
  
  int num_rows = g_board_size[player];
  int num_cols = g_board_size[player^PLAYER_MASK];

  u32bit* board = g_board[player];
  u32bit* opp_board = g_board[player^PLAYER_MASK];
    
  row = (move).array_index, col = (move).mask_index;
  horz_play = player & PLAYER_MASK;
  vert_play = player ^ PLAYER_MASK;
  
  board[row]   ^= (3<<col);
  opp_board[col]   ^= (1<<row);
  opp_board[col+1] ^= (1<<row);
  
  // update safe moves
  if(row - 1 != 0)
    _update_safe(num_rows, board, horz_play, row - 1);
  _update_safe(num_rows, board, horz_play, row);
  if(row != g_board_size[horz_play])
    _update_safe(num_rows, board, horz_play, row + 1);
  
  if(col - 1 != 0)
    _update_safe(num_cols, opp_board, vert_play, col - 1);
  if(col + 1 != g_board_size[vert_play])
    _update_safe(num_cols, opp_board, vert_play, col + 2);
  
  // update real moves
  _update_real(num_rows, board, horz_play, row);

  _update_real(num_cols, opp_board, vert_play, col);
  _update_real(num_cols, opp_board, vert_play, col + 1);
}


//#define DEBUG_SCORE_MOVE

static inline s32bit
score_move(Move move, s32bit player)
{
  s32bit row, col, horz_play, vert_play;
  s32bit score;

  int num_rows = g_board_size[player];
  int num_cols = g_board_size[player^PLAYER_MASK];

  u32bit* board = g_board[player];
  u32bit* opp_board = g_board[player^PLAYER_MASK];
    
  row = move.array_index, col = move.mask_index;
  horz_play = player & PLAYER_MASK;
  vert_play = player ^ PLAYER_MASK;
  
  board[row]   ^= (3<<col);
  opp_board[col]   ^= (1<<row);
  opp_board[col+1] ^= (1<<row);
  
  // update real moves
  score = count_real(num_rows, board, row) - g_info[horz_play][row].real;

  score -= count_real(num_cols, opp_board, col) - g_info[vert_play][col].real;
  score -= count_real(num_cols, opp_board, col + 1) - g_info[vert_play][col+1].real;

  // update safe moves
  if(row - 1 != 0)
    score +=  count_safe(num_rows, board, row - 1) - g_info[horz_play][row-1].safe;
  score += count_safe(num_rows, board, row) - g_info[horz_play][row].safe;
  if(row != g_board_size[horz_play])
    score += count_safe(num_rows, board, row+1) - g_info[horz_play][row+1].safe;
  
  if(col - 1 != 0)
    score -= count_safe(num_cols, opp_board, col - 1) - g_info[vert_play][col-1].safe;
  if(col + 1 != g_board_size[vert_play])
    score -= count_safe(num_cols, opp_board, col + 2) - g_info[vert_play][col+2].safe;
  
  g_board[horz_play][row]   ^= (3<<col);
  g_board[vert_play][col]   ^= (1<<row);
  g_board[vert_play][col+1] ^= (1<<row);
  
  score *= 128;
  score += g_first_move[player][row][col];

  return score;
}

extern void
score_and_get_first(Move movelist[MAXMOVES], s32bit num_moves,
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
        movelist[i].info = score_move(movelist[i], player);
      }
    }
  }

  else {
    for(i = 0; i < num_moves; i++){
      movelist[i].info = score_move(movelist[i], player);
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
