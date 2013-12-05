#include "positional-values.h"

//=================================================================
// Set the value of all the positions on the board.
//  We only do this once at the start of the search.
//=================================================================
PositionalValues::PositionalValues(int num_rows, int num_cols) {
  int count;
  
  // set them all to zero.
  for(int i = 0; i < 32; i++)
    for(int j = 0; j < 32; j++)
      positional_values_[i][j] = 0;

  count = 127;
  for(int j = 2; j < (num_rows+3)/2; j+=2){
    for(int k = 1; k < (num_cols/2)+1; k+=2){
       SetValue(num_rows, num_cols, j, k, --count);
    }
  }
  count = 90;
  for(int j = 2; j < (num_rows+3)/2; j+=2){
    for(int k = 1; k < (num_cols/2)+1; k++){
      if(positional_values_[j][k] == 0)
        SetValue(num_rows, num_cols, j, k, --count);
    }
  }
  count = 70;
  for(int j = 3; j < (num_rows+3)/2; j++){
    for(int k = 1; k < (num_cols/2)+1; k+=2){
      if(positional_values_[j][k] == 0)
        SetValue(num_rows, num_cols, j, k, --count);
    }
  }
  count = 50;
  for(int j = 3; j < (num_rows+3)/2; j++){
    for(int k = 1; k < (num_cols/2)+1; k++){
      if(positional_values_[j][k] == 0)
        SetValue(num_rows, num_cols, j, k, --count);
    }
  }
  count = 30;
  for(int j = 1; j < (num_rows+3)/2; j++){
    for(int k = 1; k < (num_cols/2)+1; k++){
      if(positional_values_[j][k] == 0)
        SetValue(num_rows, num_cols, j, k, --count);
    }
  }
}

void PositionalValues::SetValue(int num_rows, int num_cols,
                                int row, int col, int value) {
  positional_values_[row][col] = value;
  positional_values_[num_rows+1-row][col] = value;
  // num_cols doesn't need +1 since pieces are 2 blocks wide.
  positional_values_[num_rows+1-row][num_cols-col] = value;
  positional_values_[row][num_cols-col] = value;
}

void PositionalValues::Print() const {
    for(int j = 0; j < 32; j++){
      for(int k = 0; k < 32; k++){
        printf("%d ", positional_values_[j][k]);
      }
      printf("\n");
    }
    printf("\n");
    printf("\n");
}

#if 0
//#################################################################
// If we are using dynamic position values then these are the
//  functions (and variables) which we use to set these values.
//#################################################################

//=================================================================
// Variable to keep track of value we will give to the next position.
//=================================================================
static s32bit set_move_value_current = 127;

//=================================================================
// Initialize all positions with a value of 0
//=================================================================
extern void
init_move_value     ()
{
  s32bit i, j, k;
  set_move_value_current = 127;
  
  for(i = 0; i < 2; i++)
    for(j = 0; j < 32; j++)
      for(k = 0; k < 32; k++)
        g_first_move[i][j][k] = 0;
}

//=================================================================
// Set the value of the positions symetrical to MOVE.
//  return 0 if these positions already have a value, otherwise 1.
//=================================================================
extern s32bit
set_move_value      (Move move, s32bit player)
{
  if(g_first_move[player][move.array_index][move.mask_index] != 0)
    return 0;
  
  set_position_value(player, move.array_index, move.mask_index,
                     set_move_value_current--);
  return 1;
}

//=================================================================
// UnSet the value of the positions symetrical to MOVE.
//  Shouldn't be called if when 'set' was called it returned a value of 0.
//=================================================================
extern void
unset_move_value    (Move move, s32bit player)
{
  set_move_value_current
    = g_first_move[player][move.array_index][move.mask_index];
  
  set_position_value(player, move.array_index, move.mask_index, 0);
}
*/

#endif

