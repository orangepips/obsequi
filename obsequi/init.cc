#include "globals.h"

Board* g_boardx[2]; 
s32bit g_board_size[2] = {-1,-1};

// zobrist value for each position on the board.
s32bit       g_zobrist[32][32];

// Transposition table.
Hash_Entry  *g_trans_table = 0;

// Current boards hash key and code (flipped in various ways).
Hash_Key     norm_hashkey;
Hash_Key     flipV_hashkey;
Hash_Key     flipH_hashkey;
Hash_Key     flipVH_hashkey;

extern void
initialize_solver()
{
  s32bit i, j;

  if(g_trans_table == NULL){

    // first time initialization stuff.
    g_trans_table = (Hash_Entry*)malloc(HASHSIZE*sizeof(Hash_Entry));
  
    // initialize zobrist values
    srandom(1);
    for(i=1; i<31; i++) {
      for(j=1; j<31; j++) {
        g_zobrist[i][j] = random() & HASHMASK;
      }
    }

    init_static_tables();
  }

  init_less_static_tables();
}

static void
init_hashkey_code(Hash_Key* key) {
  int n_rows = g_boardx[HORIZONTAL]->GetNumRows();
  int n_cols = g_boardx[VERTICAL]->GetNumRows();

  key->code = 0;
  
  for(int i = 0; i < n_rows; i++)
    for(int j = 0; j < n_cols; j++){
      int index = (i*n_cols) + j;
      if(key->key[index/32] & NTH_BIT(index%32))
        key->code ^= g_zobrist[i+1][j+1];
    }
}

extern void
initialize_board(s32bit num_rows, s32bit num_cols, s32bit board[30][30])
{
  // Check if we need to re-initialize the solver.
  bool init = 1; //(g_trans_table == NULL || !horz || !vert ||
              // horz->num_rows != num_rows || vert->num_rows != num_cols);

  g_board_size[HORIZONTAL] = num_rows;
  g_board_size[VERTICAL]   = num_cols;

  Board* horz = g_boardx[HORIZONTAL] = new Board(num_rows, num_cols);
  g_boardx[VERTICAL] = horz->GetOpponent();

  if(init) initialize_solver();

  for(int i = 0; i < 4; i++){
    g_norm_hashkey.key[i]   = 0;
    g_flipV_hashkey.key[i]  = 0;
    g_flipH_hashkey.key[i]  = 0;
    g_flipVH_hashkey.key[i] = 0;
  }

  // Modify hashkeys to deal with positions which are already occupied.
  for(int i = 0; i < num_rows; i++)
    for(int j = 0; j < num_cols; j++)
      if(board[i][j] != 0){
        s32bit index;

        index = (i*num_cols)+j;
        g_norm_hashkey.key[index/32] |= NTH_BIT(index%32);
        
        index = (i*num_cols)                  + (num_cols - j - 1);
        g_flipV_hashkey.key[index/32] |= NTH_BIT(index%32);
        
        index = ( (num_rows - i - 1) *num_cols) + j;
        g_flipH_hashkey.key[index/32] |= NTH_BIT(index%32);
        
        index = ( (num_rows - i - 1) *num_cols) + (num_cols - j - 1);
        g_flipVH_hashkey.key[index/32] |= NTH_BIT(index%32);
      }

  init_hashkey_code(&g_norm_hashkey);
  init_hashkey_code(&g_flipV_hashkey);
  init_hashkey_code(&g_flipH_hashkey);
  init_hashkey_code(&g_flipVH_hashkey);

  horz->Print();
  printf("\n");
  horz->PrintInfo();//print_board_info(HORIZONTAL);

  check_hash_code_sanity();
}
