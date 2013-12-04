#include "globals.h"
#include "macros.h"

static FILE *trait_file = NULL;

static int
tr_total_non_safe_moves(int num_rows, u32bit board[MAX_ROWS]) {
  int count = 0;
  
  for(int i = 0; i < num_rows; i++){
    u32bit mask1 = board[i] & board[i+2];
    u32bit mask2 = ~board[i+1];
    
    mask1 = (~((mask1 >> 1) & mask1)) & ((mask2 >> 1) & mask2);

    count += countbits32(mask1);
  }
  
  return count;
}

static int
tr_non_safe_moves_a_little_touchy(
int num_rows, u32bit board[MAX_ROWS]) {
  int count = 0;

  for(int i = 0; i < num_rows; i++){
    u32bit mask1 = board[i] | board[i+2];
    u32bit mask2 = board[i+1];
    
    mask1 = ( (mask1 << 1) | mask1
              | (mask2 >> 1) | mask2 | (mask2 << 1) | (mask2 << 2) );

    count += countbits32(~mask1);
  }

  return count;
}

static s32bit
tr_non_safe_moves_no_touchy(s32bit player)
{
  return 1;
}

static int
tr_total_empty_squares(int num_rows, u32bit board[MAX_ROWS]) {
  int count = 0;
  
  for(int i = 0; i < num_rows; i++){
    u32bit mask = ~(board[i+1]);
    count += countbits32(mask);
  }
    
  return count;
}

static int
tr_border_length_col(int num_rows, u32bit board[MAX_ROWS]) {
  int count = 0;
  
  for (int i = 0; i <= num_rows; i++){
    u32bit mask = board[i] ^ board[i+1];
    count += countbits32(mask);
  }
  
  return count;
}

static int
tr_border_length_row(int num_rows, u32bit board[MAX_ROWS]) {
  int count = 0;
  
  for (int i = 0; i <= num_rows; i++){
    u32bit mask  = (board[i+1]>>1) ^ board[i+1];
    mask &= 0x7FFFFFFF;
    
    count += countbits32(mask);
  }
  
  return count;
}




extern void
write_node_info(u64bit num_nodes, s32bit winner)
{
  s32bit num;
  
  if(trait_file == NULL) {
    trait_file = fopen("trait_file", "w");
    if(trait_file == NULL)
      fprintf(stderr, "Couldn't open \"trait_file\".\n");
  }
  
  fprintf(trait_file, "%c %15s :", (winner == VERTICAL) ? 'V' : 'H',
          u64bit_to_string(num_nodes));
  
  //========================================================
  // number of non-safe moves.
  //========================================================
  
  int rows = g_board_size[winner];
  int cols = g_board_size[winner^PLAYER_MASK];

  u32bit* w_board = g_board[winner];    
  u32bit* opp_board = g_board[winner^PLAYER_MASK];    

  // total number allowing overlapping of other non-safe moves
  //   and of safe moves.
  num = tr_total_non_safe_moves(rows, w_board);
  fprintf(trait_file, " %2d", num);

  num = tr_total_non_safe_moves(cols, opp_board);
  fprintf(trait_file, " %2d :", num);
  
  
  // non safe moves that don't touch a border, except diagonally.
  num = tr_non_safe_moves_a_little_touchy(rows, w_board);
  fprintf(trait_file, " %2d", num);

  num = tr_non_safe_moves_a_little_touchy(cols, opp_board);
  fprintf(trait_file, " %2d :", num);


  // non safe moves that don't touch a border, never.
  num = tr_non_safe_moves_no_touchy(winner);
  fprintf(trait_file, " %2d", num);

  num = tr_non_safe_moves_no_touchy(winner^PLAYER_MASK);
  fprintf(trait_file, " %2d :", num);


  //========================================================
  // Miscellaneous other info.
  //========================================================

  // empty squares.
  num = tr_total_empty_squares(rows, w_board);
  fprintf(trait_file, " %2d :", num);

  // border length.
  num = tr_border_length_col(rows, w_board);
  fprintf(trait_file, " %2d", num);

  // border length.
  num = tr_border_length_row(rows, w_board);
  fprintf(trait_file, " %2d", num);

  fprintf(trait_file, "\n");
}
