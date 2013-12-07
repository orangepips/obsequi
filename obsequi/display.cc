#include "globals.h"

//========================================================
// This function compares the Horizontal board info to
//  the vertical board info.
//========================================================
/*
  extern void
check_board_sanity()
{
  s32bit i, j;
  s32bit count;

  for(j = 0; j < g_board_size[HORIZONTAL] + 2; j++)
    for(i = 0; i < g_board_size[VERTICAL] + 2; i++){
      count = 0;

      if(g_board[VERTICAL][i] & NTH_BIT(j)) count++;
      if(g_board[HORIZONTAL][j] & NTH_BIT(i)) count++;

      if(count == 1){
        print_board(VERTICAL);
        print_board(HORIZONTAL);

        printf("%d %d - %d.\n", j, i, count);

        fatal_error(1, "Board is inconsistent.\n");
      }
    }
}
*/

//========================================================
// Check the board info which we have to make sure that it
//  is accurate.
//========================================================
/*
extern void
check_info_sanity()
{
}*/



//########################################################
// Display functions.
// These functions print out some set of information in what
//   is hopefully a fairly readable format.
//########################################################
extern void
print_u64bit(u64bit val)
{
  s32bit vals[10];
  s32bit i = 0;

  do {
    vals[i] = val % 1000;
    val = val / 1000;
    i++;
  } while(val != 0);

  if(i > 10) fatal_error(1, "Too large???\n");

  printf("%d", vals[--i]);

  while(i != 0)
    printf(",%3d", vals[--i]);
}



extern const char*
current_search_state()
{
  static char* str = NULL;

  if(str != NULL) free(str);

  int x = asprintf(&str, "Nodes: %s.\n%d %d %d %d %d %d %d %d %d %d %d %d.",
                   u64bit_to_string(g_num_nodes),
                   g_move_number[0], g_move_number[1], g_move_number[2],
                   g_move_number[3], g_move_number[4], g_move_number[5],
                   g_move_number[6], g_move_number[7], g_move_number[8],
                   g_move_number[9], g_move_number[10], g_move_number[11]);
  if (x == -1) exit(3);

  return str;
}
