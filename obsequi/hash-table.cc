#include "globals.h"
#include "hash-table.h"

//########################################################
// Constants used in conjuction with the transposition table
//   and the zobrist values.
//########################################################

#ifndef HASHCODEBITS
#error "HASHCODEBITS must be defined.\n"
#endif

#if ! (HASHCODEBITS > 10 && HASHCODEBITS < 31)
#error "Invalid value for HASHCODEBITS, it should be a value from 11 - 30."
#endif

#define HASHSIZE (1 << HASHCODEBITS)
#define HASHMASK (HASHSIZE - 1)

#define LOWER   0
#define EXACT   1
#define UPPER   2

#define KEYSIZE 4

//########################################################
// Info we need for each entry in the hashtable.
//########################################################
typedef struct
{
  // uniquely identifies a board position.
  u32bit key[KEYSIZE];

  // if real num of nodes exceeds ULONG_MAX set to ULONG_MAX.
  //   or maybe we could just shift the bits (larger granularity).
  u32bit nodes;

  // uniquely identifies the previous best move for this position.
  u8bit  best_move;

  // depth of the search when this value was determined.
  u8bit  depth : 7;

  // whos turn it is.
  u8bit  whos_turn : 1;

  // value of node determined with a search to `depth`.
  s16bit value : 14;

  // value of node determined with a search to `depth`.
  u16bit type : 2; //UPPER, LOWER, EXACT.
} Hash_Entry;


//########################################################
// structure used to store current key and it's hash code.
//########################################################
typedef struct
{
  u32bit key[KEYSIZE];
  s32bit code;
} Hash_Key;


//########################################################
// table_keyinfo
//########################################################
typedef struct
{
  s32bit bit1_index;
  s32bit bit2_index;
  u32bit hash_code;
} KeyInfo_s;

typedef struct
{
  KeyInfo_s norm;
  KeyInfo_s flipV;
  KeyInfo_s flipH;
  KeyInfo_s flipVH;
} KeyInfo;


//========================================================
// Global variables, lets get rid of these.
//========================================================
KeyInfo      g_keyinfo[2][32][32];

// zobrist value for each position on the board.
s32bit       g_zobrist[32][32];

// Transposition table.
Hash_Entry  *g_trans_table;

// Current boards hash key and code (flipped in various ways).
Hash_Key     g_norm_hashkey;
Hash_Key     g_flipV_hashkey;
Hash_Key     g_flipH_hashkey;
Hash_Key     g_flipVH_hashkey;


// ##################################################################
// print_external
// ##################################################################

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
  print_keyinfo_table(HORIZONTAL, 1);
  print_keyinfo_table(VERTICAL, 1);
}


// ##################################################################
// print_hashentry
// ##################################################################

extern void
print_hashentry(s32bit index)
{
  Hash_Entry entry = g_trans_table[index];
  
  printf("Hash entry: %d.\n", index);
  printf(" Key:%8X:%8X:%8X:%8X, n:%u, d:%d, w:%d"
         ", v:%d, t:%d, int:%d.\n",
         entry.key[0], entry.key[1], entry.key[2], entry.key[3],
         entry.nodes, entry.depth, 0, //entry.whos_turn,
         entry.value, entry.type, entry.best_move);
}


// ##################################################################
// init_hashtable
// ##################################################################
static void
negate_keyinfo(KeyInfo_s *keyinfo)
{
  keyinfo->bit1_index = keyinfo->bit2_index = -1;
  keyinfo->hash_code = 0;
}
 
static void
fill_in_hash_code(KeyInfo_s *info, s32bit num_cols)
{
  s32bit r, c, hash = 0;
    
  r = info->bit1_index/num_cols;
  c = info->bit1_index%num_cols;
  
  hash = g_zobrist[r+1][c+1];
  
  r = info->bit2_index/num_cols;
  c = info->bit2_index%num_cols;
  
  hash ^= g_zobrist[r+1][c+1];

  info->hash_code = hash;
}

static void
fill_in_key_entry(KeyInfo *keyinfo, s32bit num_rows, s32bit num_cols)
{
  if(keyinfo->norm.bit1_index == -1){
    negate_keyinfo( & keyinfo->norm);
    negate_keyinfo( & keyinfo->flipV);
    negate_keyinfo( & keyinfo->flipH);
    negate_keyinfo( & keyinfo->flipVH);
  } else {
    s32bit r1, c1, r2, c2;
    
    r1 = keyinfo->norm.bit1_index/num_cols;
    c1 = keyinfo->norm.bit1_index%num_cols;
    r2 = keyinfo->norm.bit2_index/num_cols;
    c2 = keyinfo->norm.bit2_index%num_cols;
  
    keyinfo->flipV.bit1_index  = (r1*num_cols)+(num_cols - c1 - 1);
    keyinfo->flipV.bit2_index  = (r2*num_cols)+(num_cols - c2 - 1);

    keyinfo->flipH.bit1_index  = ((num_rows - r1 - 1) * num_cols) + c1;
    keyinfo->flipH.bit2_index  = ((num_rows - r2 - 1) * num_cols) + c2;

    keyinfo->flipVH.bit1_index = ( ((num_rows - r1 - 1) * num_cols)
                                   + (num_cols - c1 - 1) );
    keyinfo->flipVH.bit2_index = ( ((num_rows - r2 - 1) * num_cols)
                                   + (num_cols - c2 - 1) );

    fill_in_hash_code( & keyinfo->norm, num_cols);
    fill_in_hash_code( & keyinfo->flipV, num_cols);
    fill_in_hash_code( & keyinfo->flipH, num_cols);
    fill_in_hash_code( & keyinfo->flipVH, num_cols);
  }
}

extern void
init_less_static_tables()
{
  s32bit n_rows, n_cols, i, j, k;
  
  n_rows = g_board_size[HORIZONTAL], n_cols = g_board_size[VERTICAL];
  
  for(i = 0; i < 32; i++)
    for(j = 0; j < 32; j++)
      for(k = 0; k < 2; k++)
        negate_keyinfo( & g_keyinfo[k][i][j].norm);
  
  for(i = 0; i < n_rows; i++){
    for(j = 0; j < n_cols; j++){
      //Horizontal Entry
      if(j + 1 < n_cols){
        g_keyinfo[HORIZONTAL][i+1][j+1].norm.bit1_index = (i*n_cols)+j;
        g_keyinfo[HORIZONTAL][i+1][j+1].norm.bit2_index = (i*n_cols)+(j+1);
      }
      
      //Vertical Entry
      if(i + 1 < n_rows){
        g_keyinfo[VERTICAL][j+1][i+1].norm.bit1_index   = (i*n_cols)+j;
        g_keyinfo[VERTICAL][j+1][i+1].norm.bit2_index   = ((i+1)*n_cols)+j;
      }
    }
  }
  
  for(i = 0; i < 32; i++)
    for(j = 0; j < 32; j++)
      for(k = 0; k < 2; k++)
        fill_in_key_entry(&g_keyinfo[k][i][j], n_rows, n_cols);
}

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
init_hashtable(s32bit num_rows, s32bit num_cols, s32bit board[30][30])
{
  bool init = 1;
  if(init) initialize_solver();

  for(int i = 0; i < KEYSIZE; i++){
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
        
        index = (i*num_cols) + (num_cols - j - 1);
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

  check_hash_code_sanity();
}

//########################################################
// check_hash_code_sanity
//########################################################
extern void
print_hashkey(Hash_Key key)
{
  printf("Key: %8X:%8X:%8X:%8X, Code: %8X.\n",
         key.key[0], key.key[1], key.key[2], key.key[3], key.code);
}

static void
check_hashkey_bit_set(Hash_Key key, s32bit index)
{
  if(! (key.key[index/32] & NTH_BIT(index%32)) ){

    printf("%d", index);
    
    print_hashkey(key);

    fatal_error(1, "HashKey Incorrect.\n");
  }
}

static void
check_hashkey_bit_not_set(Hash_Key key, s32bit index)
{
  if( (key.key[index/32] & NTH_BIT(index%32)) ){

    printf("%d", index);
    
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
        
        index = (i*n_cols) + (n_cols - j - 1);
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
// toggle_hash_code
//========================================================
#define HASHKEY_UPDATE(key,index) key[index/32] ^= NTH_BIT(index%32)

void toggle_hash_code(int whos_turn, const Move& move) {
  KeyInfo info = g_keyinfo[whos_turn][move.array_index][move.mask_index];

  HASHKEY_UPDATE(g_norm_hashkey.key, info.norm.bit1_index);
  HASHKEY_UPDATE(g_norm_hashkey.key, info.norm.bit2_index);
  g_norm_hashkey.code ^= info.norm.hash_code;

  HASHKEY_UPDATE(g_flipV_hashkey.key, info.flipV.bit1_index);
  HASHKEY_UPDATE(g_flipV_hashkey.key, info.flipV.bit2_index);
  g_flipV_hashkey.code ^= info.flipV.hash_code;

  HASHKEY_UPDATE(g_flipH_hashkey.key, info.flipH.bit1_index);
  HASHKEY_UPDATE(g_flipH_hashkey.key, info.flipH.bit2_index);
  g_flipH_hashkey.code ^= info.flipH.hash_code;

  HASHKEY_UPDATE(g_flipVH_hashkey.key, info.flipVH.bit1_index);
  HASHKEY_UPDATE(g_flipVH_hashkey.key, info.flipVH.bit2_index);
  g_flipVH_hashkey.code ^= info.flipVH.hash_code;
}


//########################################################
// hashstore
//########################################################

#define MOVE_TO_INT(mv, player)   \
   ((mv).mask_index-1)*g_board_size[(player)]+((mv).array_index-1)

#define TABLE_SET_KEY(table,index,k)                          \
   table[index].key[0] = k[0], table[index].key[1] = k[1],    \
   table[index].key[2] = k[2], table[index].key[3] = k[3]; 

#define TABLE_CMP_KEY(table,index,k)                              \
   (table[index].key[0] == k[0] && table[index].key[1] == k[1] && \
    table[index].key[2] == k[2] && table[index].key[3] == k[3]) 


#define STORE_ENTRY(x)                                          \
  index = (x).code;                                             \
                                                                \
  if(TABLE_CMP_KEY(g_trans_table, index, (x).key)               \
     || g_trans_table[index].nodes <= nodes){                   \
    TABLE_SET_KEY(g_trans_table, index, (x).key);               \
    g_trans_table[index].nodes     =nodes;                      \
    g_trans_table[index].best_move =MOVE_TO_INT(best,player);   \
    g_trans_table[index].depth     =depth;                      \
    g_trans_table[index].whos_turn =player;                     \
    g_trans_table[index].value     =value;                      \
    if     (value>=beta) g_trans_table[index].type =LOWER;      \
    else if(value>alpha) g_trans_table[index].type =EXACT;      \
    else                 g_trans_table[index].type =UPPER;      \
    return;                                                     \
  }


extern void
hashstore(s32bit value, s32bit alpha, s32bit beta,
          u32bit nodes, s32bit depth, Move best, s32bit player)
{
  s32bit index;

  STORE_ENTRY(g_norm_hashkey);
  STORE_ENTRY(g_flipV_hashkey);
  STORE_ENTRY(g_flipH_hashkey);
  STORE_ENTRY(g_flipVH_hashkey);
}


//########################################################
// hashlookup
//########################################################

#define INT_TO_MOVE(mv, int, player)                    \
   (mv).mask_index = ((int)/g_board_size[(player)])+1;  \
   (mv).array_index = ((int)%g_board_size[(player)])+1


static inline int
hash_lookup_entry(const Hash_Key& key, s32bit *value,
                  s32bit *alpha, s32bit *beta,
                  s32bit depth_remaining, Move *force_first,
                  s32bit player) {
  int index = key.code;
  if(TABLE_CMP_KEY(g_trans_table, index, key.key)
     && g_trans_table[index].whos_turn == player ) {
    /* found matching entry.*/

    /* If nothing else we can use this entry to give us a good move. */
    INT_TO_MOVE(*force_first, g_trans_table[index].best_move, player);

    /* use value if depth >= than the depth remaining in our search. */
    if(g_trans_table[index].depth >= depth_remaining) {

      /* if the value is exact we can use it. */
      if(g_trans_table[index].type == EXACT) {
        *value=g_trans_table[index].value;
        return 1;
      }

      /* if value is a lower bound we can possibly use it. */
      if(g_trans_table[index].type == LOWER) {
        if(g_trans_table[index].value>=(*beta)){
          *value=g_trans_table[index].value;
          return 1;
        }
        if(g_trans_table[index].value>(*alpha)){
          *alpha=g_trans_table[index].value;
        }
        return 0;
      }

      /* if value is a upper bound we can possibly use it. */
      if(g_trans_table[index].type == UPPER) {
        if(g_trans_table[index].value<=(*alpha)){
          *value=g_trans_table[index].value;
          return 1;
        }
        if(g_trans_table[index].value<(*beta)){
          *beta=g_trans_table[index].value;
        }
        return 0;
      }
    }
  }
  return -1;
}

extern s32bit
hashlookup(s32bit *value, s32bit *alpha, s32bit *beta,
           s32bit depth_remaining, Move *force_first, s32bit player)
{
  int rv;

  rv = hash_lookup_entry(g_norm_hashkey, value, alpha, beta,
                  depth_remaining, force_first, player);
  if (rv != -1) return rv;
    
  rv = hash_lookup_entry(g_flipV_hashkey, value, alpha, beta,
                  depth_remaining, force_first, player);
  if (rv != -1) return rv;
    
  rv = hash_lookup_entry(g_flipH_hashkey, value, alpha, beta,
                  depth_remaining, force_first, player);
  if (rv != -1) return rv;
    
  rv = hash_lookup_entry(g_flipVH_hashkey, value, alpha, beta,
                  depth_remaining, force_first, player);
  if (rv != -1) return rv;
    
  return 0;
}
