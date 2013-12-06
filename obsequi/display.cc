

#include "globals.h"

//########################################################
// Sanity check functions.
// These functions try to do a quick check of some set of
//   information. They crash and burn if the check fails.
//########################################################

static void
check_hashkey_bit_set(Hash_Key key, s32bit index)
{
  if(! (key.key[index/32] & NTH_BIT(index%32)) ){

    printf("%d", index);
    
    print_bitboard(HORIZONTAL);

    print_hashkey(key);

    fatal_error(1, "HashKey Incorrect.\n");
  }
}

static void
check_hashkey_bit_not_set(Hash_Key key, s32bit index)
{
  if( (key.key[index/32] & NTH_BIT(index%32)) ){

    printf("%d", index);
    
    print_bitboard(HORIZONTAL);

    print_hashkey(key);

    fatal_error(1, "HashKey Incorrect.\n");
  }
}

static void
check_hashkey_code(Hash_Key key)
{
  s32bit i, j, index, code;
  
  int n_rows = g_boardx[HORIZONTAL]->GetNumRows();
  int n_cols = g_boardx[VERTICAL]->GetNumRows();

  code = key.code;
  
  for(i = 0; i < n_rows; i++)
    for(j = 0; j < n_cols; j++){
      index = (i*n_cols) + j;
      if(key.key[index/32] & NTH_BIT(index%32))
        code ^= g_zobrist[i+1][j+1];
    }
  
  if(code != 0){
    fatal_error(1, "Invalid hash code.\n");
  }
}



//========================================================
// This function checks to make sure hash codes
//   and keys are correct.
//========================================================
extern void
check_hash_code_sanity()
{
  s32bit i, j, index;
  
  int n_rows = g_boardx[HORIZONTAL]->GetNumRows();
  int n_cols = g_boardx[VERTICAL]->GetNumRows();
  
  for(i = 0; i < n_rows; i++)
    for(j = 0; j < n_cols; j++)
      if(g_boardx[HORIZONTAL]->board[i+1] & NTH_BIT(j+1)) {

        index = (i*n_cols)+j;
        check_hashkey_bit_set(g_norm_hashkey, index);
        
        index = (i*n_cols)                  + (n_cols - j - 1);
        check_hashkey_bit_set(g_flipV_hashkey, index);
        
        index = ( (n_rows - i - 1) *n_cols) + j;
        check_hashkey_bit_set(g_flipH_hashkey, index);
        
        index = ( (n_rows - i - 1) *n_cols) + (n_cols - j - 1);
        check_hashkey_bit_set(g_flipVH_hashkey, index);
      } else {
        index = (i*n_cols)+j;
        check_hashkey_bit_not_set(g_norm_hashkey, index);
        
        index = (i*n_cols)                  + (n_cols - j - 1);
        check_hashkey_bit_not_set(g_flipV_hashkey, index);
        
        index = ( (n_rows - i - 1) *n_cols) + j;
        check_hashkey_bit_not_set(g_flipH_hashkey, index);
        
        index = ( (n_rows - i - 1) *n_cols) + (n_cols - j - 1);
        check_hashkey_bit_not_set(g_flipVH_hashkey, index);
      }

  check_hashkey_code(g_norm_hashkey);
  check_hashkey_code(g_flipV_hashkey);
  check_hashkey_code(g_flipH_hashkey);
  check_hashkey_code(g_flipVH_hashkey);
}



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
print_hashentry(s32bit index)
{
  Hash_Entry entry = g_trans_table[index];
  
  printf("Hash entry: %d.\n", index);
  printf(" Key:%8X:%8X:%8X:%8X, n:%u, d:%d, w:%d"
         ", v:%d, t:%d, int:%d.\n",
         entry.key[0], entry.key[1], entry.key[2], entry.key[3],
         entry.nodes, entry.depth, entry.whos_turn,
         entry.value, entry.type, entry.best_move);
}

extern void
print_bitboard(s32bit player)
{
}

extern void
print_hashkey(Hash_Key key)
{
  printf("Key: %8X:%8X:%8X:%8X, Code: %8X.\n",
         key.key[0], key.key[1], key.key[2], key.key[3], key.code);
}

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

extern void
print_keyinfo(KeyInfo_s info, s32bit with)
{
  if(info.bit1_index == -1) fatal_error(1, "bit1_index equal to -1");

  if(with)
    printf("%3d:%3d %8X ",
           info.bit1_index, info.bit2_index, info.hash_code);
  else
    printf("%3d:%3d ",
           info.bit1_index, info.bit2_index);
}


extern void
print_keyinfo_table(s32bit player, s32bit with_hashcode)
{
  s32bit r, c;
  
  for(r = 0; r < 32; r++)
    for(c = 0; c < 32; c++){
      if(g_keyinfo[player][r][c].norm.bit1_index != -1){
      
        printf("(%2d,%2d)>>  ", r, c);

        print_keyinfo(g_keyinfo[player][r][c].norm, with_hashcode);
        print_keyinfo(g_keyinfo[player][r][c].flipV, with_hashcode);
        print_keyinfo(g_keyinfo[player][r][c].flipH, with_hashcode);
        print_keyinfo(g_keyinfo[player][r][c].flipVH, with_hashcode);
        putchar('\n');
      }
    }
}


extern void
print_external()
{
  //  print_keyinfo_table(HORIZONTAL, 1);
  print_keyinfo_table(VERTICAL, 1);
}

extern s32bit g_print;

/*
extern void
print_current_state()
{
  print_board(HORIZONTAL);
  printf("\n");
  print_board_info(HORIZONTAL);
  g_print = 0;
}
*/


extern const char*
current_search_state()
{
  static char* str = NULL;

  g_print = 1;
      
  if(str != NULL) free(str);
 
  int x = asprintf(&str, "Nodes: %s.\n%d %d %d %d %d %d %d %d %d.",
                   u64bit_to_string(g_num_nodes),
                   g_move_number[1], g_move_number[2], g_move_number[3],
                   g_move_number[4], g_move_number[5], g_move_number[6],
                   g_move_number[7], g_move_number[8], g_move_number[9]);
  if (x == -1) exit(3);

  return str;
}
