#include "hash-table.h"

#include "globals.h"

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

#define FLIP_NORMAL 0
#define FLIP_VERT 1
#define FLIP_HORZ 2
#define FLIP_VERT_HORZ 3
#define FLIP_TOTAL 4


//########################################################
// Info we need for each entry in the hashtable.
//########################################################
struct Hash_Entry {
  // uniquely identifies a board position.
  u64bit key[HASH_KEY_SIZE];

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
};

//########################################################
// structure used to store current key and it's hash code.
//########################################################
struct Hash_Key {
  u64bit key[HASH_KEY_SIZE];
  u32bit code;
};

//########################################################
// table_keyinfo
//########################################################
struct KeyInfo {
  Hash_Key mod[FLIP_TOTAL];
};


//========================================================
// Global variables, lets get rid of these.
//========================================================
// zobrist value for each position on the board.
u32bit       g_zobrist[32][32];

// KeyInfo
KeyInfo      g_keyinfo[2][32][32];

// Transposition table.
Hash_Entry  *g_trans_table;

// Current boards hash key and code (flipped in various ways).
Hash_Key     g_hashkey[FLIP_TOTAL];


// ##################################################################
// init_hashtable
// ##################################################################
static void fill_in_hash_code(Hash_Key *info, int num_cols, int bit) {
  int r = bit / num_cols;
  int c = bit % num_cols;
  info->code ^= g_zobrist[r+1][c+1];
  info->key[bit/64] ^= NTH_BIT(bit%64);
}

static void fill_in_key_entry(KeyInfo *keyinfo, int bit1, int bit2,
                       int num_rows, int num_cols) {
  int r1 = bit1/num_cols;
  int c1 = bit1%num_cols;
  int r2 = bit2/num_cols;
  int c2 = bit2%num_cols;

  fill_in_hash_code(&keyinfo->mod[FLIP_NORMAL], num_cols, bit1);
  fill_in_hash_code(&keyinfo->mod[FLIP_NORMAL], num_cols, bit2);

  fill_in_hash_code(&keyinfo->mod[FLIP_VERT], num_cols,
      (r1*num_cols)+(num_cols - c1 - 1));
  fill_in_hash_code(&keyinfo->mod[FLIP_VERT], num_cols,
      (r2*num_cols)+(num_cols - c2 - 1));

  fill_in_hash_code(&keyinfo->mod[FLIP_HORZ], num_cols,
      ((num_rows - r1 - 1) * num_cols) + c1);
  fill_in_hash_code(&keyinfo->mod[FLIP_HORZ], num_cols,
      ((num_rows - r2 - 1) * num_cols) + c2);

  fill_in_hash_code(&keyinfo->mod[FLIP_VERT_HORZ], num_cols,
      ( ((num_rows - r1 - 1) * num_cols) + (num_cols - c1 - 1) ));
  fill_in_hash_code(&keyinfo->mod[FLIP_VERT_HORZ], num_cols,
      ( ((num_rows - r2 - 1) * num_cols) + (num_cols - c2 - 1) ));
}

void init_hashtable(s32bit num_rows, s32bit num_cols, s32bit board[30][30]) {
  // initialize zobrist values
  srandom(1);
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
      g_zobrist[i][j] = random() & HASHMASK;
    }
  }

  // Initialize g_keyinfo
  for (int i = 0; i < 32 - 1; i++) {
    for (int j = 0; j < 32 - 1; j++) {
      fill_in_key_entry(&g_keyinfo[HORIZONTAL][i+1][j+1],
          (i*num_cols)+j, (i*num_cols)+j+1, num_rows, num_cols);
      fill_in_key_entry(&g_keyinfo[VERTICAL][j+1][i+1],
          (i*num_cols)+j, ((i+1)*num_cols)+j, num_rows, num_cols);
    }
  }

  // Initialize trans_table
  g_trans_table = new Hash_Entry[HASHSIZE];

  // Modify hashkeys to deal with positions which are already occupied.
  for(int i = 0; i < num_rows; i++) {
    for(int j = 0; j < num_cols; j++) {
      if(board[i][j] != 0){
        fill_in_hash_code(&g_hashkey[FLIP_NORMAL], num_cols,
            (i*num_cols)+j);
        fill_in_hash_code(&g_hashkey[FLIP_VERT], num_cols,
            (i*num_cols) + (num_cols - j - 1));
        fill_in_hash_code(&g_hashkey[FLIP_HORZ], num_cols,
            ( (num_rows - i - 1) *num_cols) + j);
        fill_in_hash_code(&g_hashkey[FLIP_VERT_HORZ], num_cols,
            ( (num_rows - i - 1) *num_cols) + (num_cols - j - 1));
      }
    }
  }

  check_hash_code_sanity();
}


// ##################################################################
// print_hashentry
// ##################################################################
void print_hashentry(s32bit index) {
  Hash_Entry entry = g_trans_table[index];

  printf("Hash entry: %d.\n", index);
  printf(" Key:%8lX:%8lX, n:%u, d:%d, w:%d"
         ", v:%d, t:%d, int:%d.\n",
         entry.key[0], entry.key[1],
         entry.nodes, entry.depth, 0, //entry.whos_turn,
         entry.value, entry.type, entry.best_move);
}


//########################################################
// check_hash_code_sanity
//########################################################
extern void
print_hashkey(Hash_Key key)
{
  printf("Key: %8lX:%8lX, Code: %8X.\n",
         key.key[0], key.key[1], key.code);
}

static void
check_hashkey_bit_set(Hash_Key key, s32bit index)
{
  if(! (key.key[index/64] & NTH_BIT(index%64)) ){

    printf("%d", index);

    print_hashkey(key);

    fatal_error(1, "HashKey Incorrect.\n");
  }
}

static void
check_hashkey_bit_not_set(Hash_Key key, s32bit index)
{
  if( (key.key[index/64] & NTH_BIT(index%64)) ){

    printf("%d", index);

    print_hashkey(key);

    fatal_error(1, "HashKey Incorrect.\n");
  }
}

static void
check_hashkey_code(Hash_Key key, int n_rows, int n_cols) {
  int code = key.code;
  for (int i = 0; i < n_rows; i++)
    for (int j = 0; j < n_cols; j++){
      int index = (i*n_cols) + j;
      if(key.key[index/64] & NTH_BIT(index%64))
        code ^= g_zobrist[i+1][j+1];
    }

  if(code != 0){
    fatal_error(1, "Invalid hash code.\n");
  }
}

extern void
check_hash_code_sanity() {
  int n_rows = g_boardx[HORIZONTAL]->GetNumRows();
  int n_cols = g_boardx[VERTICAL]->GetNumRows();

  for (int i = 0; i < n_rows; i++)
    for (int j = 0; j < n_cols; j++) {
      // XX
      int flipx =  (i                * n_cols) + j;
      int flipv =  (i                * n_cols) + (n_cols - j - 1);
      int fliph =  ((n_rows - i - 1) * n_cols) + j;
      int flipvh = ((n_rows - i - 1) * n_cols) + (n_cols - j - 1);
      if(g_boardx[HORIZONTAL]->board[i+1] & NTH_BIT(j+1)) {
        check_hashkey_bit_set(g_hashkey[FLIP_NORMAL], flipx);
        check_hashkey_bit_set(g_hashkey[FLIP_VERT], flipv);
        check_hashkey_bit_set(g_hashkey[FLIP_HORZ], fliph);
        check_hashkey_bit_set(g_hashkey[FLIP_VERT_HORZ], flipvh);
      } else {
        check_hashkey_bit_not_set(g_hashkey[FLIP_NORMAL], flipx);
        check_hashkey_bit_not_set(g_hashkey[FLIP_VERT], flipv);
        check_hashkey_bit_not_set(g_hashkey[FLIP_HORZ], fliph);
        check_hashkey_bit_not_set(g_hashkey[FLIP_VERT_HORZ], flipvh);
      }
    }

  for (int j = 0; j < FLIP_TOTAL; j++) {
    check_hashkey_code(g_hashkey[j], n_rows, n_cols);
  }
}


//========================================================
// toggle_hash_code
//========================================================
void toggle_hash_code(int whos_turn, const Move& move) {
  KeyInfo info = g_keyinfo[whos_turn][move.array_index][move.mask_index];

  for (int i = 0; i < FLIP_TOTAL; i++) {
    for (int j = 0; j < HASH_KEY_SIZE; j++) {
      g_hashkey[i].key[j] ^= info.mod[i].key[j];
    }
    g_hashkey[i].code ^= info.mod[i].code;
  }
}


//########################################################
// hashstore
//########################################################

#define MOVE_TO_INT(mv, player)   \
   ((mv).mask_index-1)*g_board_size[(player)]+((mv).array_index-1)

#define TABLE_SET_KEY(table,index,k)                          \
   table[index].key[0] = k[0], table[index].key[1] = k[1];

#define TABLE_CMP_KEY(table,index,k)                              \
   (table[index].key[0] == k[0] && table[index].key[1] == k[1])

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
          u32bit nodes, s32bit depth, const Move& best, s32bit player)
{
  s32bit index;

  for (int j = 0; j < FLIP_TOTAL; j++) {
    STORE_ENTRY(g_hashkey[j]);
  }
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
  if (TABLE_CMP_KEY(g_trans_table, index, key.key)
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

  for (int j = 0; j < FLIP_TOTAL; j++) {
    rv = hash_lookup_entry(g_hashkey[j], value, alpha, beta,
                           depth_remaining, force_first, player);
    if (rv != -1) return rv;
  }
  return 0;
}
