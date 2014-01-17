#include "hash-table.h"

#include "globals.h"

//########################################################
// Constants used in conjuction with the transposition table
//   and the zobrist values.
//########################################################

#define HASHCODEBITS 24

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

  // NOTE: performance seems to be very sensitive on data size.

  // if real num of nodes exceeds ULONG_MAX set to ULONG_MAX.
  //   or maybe we could just shift the bits (larger granularity).
  u32bit nodes;

  // value of node determined with a search to `depth`.
  s16bit value;

  // uniquely identifies the previous best move for this position.
  //u8bit mask_index;
  //u8bit array_index;

  // depth of the search when this value was determined.
  u8bit depth : 6;

  // value of node determined with a search to `depth`.
  u8bit type : 2; //UPPER, LOWER, EXACT.
};


//========================================================
// Global variables.
//========================================================
// zobrist value for each position on the board.
u32bit* g_zobrist;

// Transposition table.
Hash_Entry* g_trans_table;


// ##################################################################
// Zobrist functionality.
// ##################################################################
u32bit get_zobrist_value(int row, int col) {
  if (!g_zobrist) {
    g_zobrist = new u32bit[32*32];
    srandom(1);
    for (int i = 0; i < 32*32; i++) {
      g_zobrist[i] = random() & HASHMASK;
    }
  }

  return g_zobrist[(row+1) * 32 + (col+1)];
}

// ##################################################################
// HashKeys class.
// ##################################################################
static void fill_in_hash_code(HashKey *info, int num_cols, int bit) {
  int r = bit / num_cols;
  int c = bit % num_cols;
  info->code ^= get_zobrist_value(r, c);
  info->key[bit/64] ^= NTH_BIT(bit%64);
}

void HashKeys::Toggle(int num_rows, int num_cols, int bit) {
  int row = bit / num_cols;
  int col = bit % num_cols;

  fill_in_hash_code(&mod[FLIP_NORMAL], num_cols, bit);
  fill_in_hash_code(&mod[FLIP_VERT], num_cols,
      (row*num_cols)+(num_cols - col - 1));
  fill_in_hash_code(&mod[FLIP_HORZ], num_cols,
      ((num_rows - row - 1) * num_cols) + col);
  fill_in_hash_code(&mod[FLIP_VERT_HORZ], num_cols,
      ( ((num_rows - row - 1) * num_cols) + (num_cols - col - 1) ));
}

void HashKeys::Init(int num_rows, int num_cols, int bit1, int bit2) {
  Toggle(num_rows, num_cols, bit1);
  Toggle(num_rows, num_cols, bit2);
}

void HashKeys::Print() const {
  for (int i = 0; i < FLIP_TOTAL; i++) {
    printf("Dir: %d, Key: %8lX:%8lX, Code: %8X.\n",
           i, mod[i].key[0], mod[i].key[1], mod[i].code);
  }
}

// ##################################################################
// init_hashtable
// ##################################################################
void init_hashtable(s32bit num_rows, s32bit num_cols, s32bit board[30][30]) {
  // Initialize trans_table
  g_trans_table = new Hash_Entry[HASHSIZE];
}


// ##################################################################
// print_hashentry
// ##################################################################
void print_hashentry(s32bit index) {
  Hash_Entry entry = g_trans_table[index];

  printf("Hash entry: %d.\n", index);
  printf(" Key:%8lX:%8lX, n:%u, d:%d, w:%d"
         ", v:%d, t:%d, int:%d,%d.\n",
         entry.key[0], entry.key[1],
         entry.nodes, entry.depth, 0, //entry.whos_turn,
         entry.value, entry.type,
         0, 0);//entry.mask_index, entry.array_index);
}


//########################################################
// check_hash_code_sanity
//########################################################
extern void
print_hashkey(HashKey key)
{
  printf("Key: %8lX:%8lX, Code: %8X.\n",
         key.key[0], key.key[1], key.code);
}

static void
check_hashkey_bit_set(HashKey key, s32bit index)
{
  if(! (key.key[index/64] & NTH_BIT(index%64)) ){

    printf("%d", index);

    print_hashkey(key);

    fatal_error(1, "HashKey Incorrect.\n");
  }
}

static void
check_hashkey_bit_not_set(HashKey key, s32bit index)
{
  if( (key.key[index/64] & NTH_BIT(index%64)) ){

    printf("%d", index);

    print_hashkey(key);

    fatal_error(1, "HashKey Incorrect.\n");
  }
}

static void
check_hashkey_code(HashKey key, int n_rows, int n_cols) {
  int code = key.code;
  for (int i = 0; i < n_rows; i++)
    for (int j = 0; j < n_cols; j++){
      int index = (i*n_cols) + j;
      if(key.key[index/64] & NTH_BIT(index%64))
        code ^= get_zobrist_value(i, j);
    }

  if(code != 0){
    fatal_error(1, "Invalid hash code.\n");
  }
}

extern void
check_hash_code_sanity(const HashKeys& keys) {
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
        check_hashkey_bit_set(keys.mod[FLIP_NORMAL], flipx);
        check_hashkey_bit_set(keys.mod[FLIP_VERT], flipv);
        check_hashkey_bit_set(keys.mod[FLIP_HORZ], fliph);
        check_hashkey_bit_set(keys.mod[FLIP_VERT_HORZ], flipvh);
      } else {
        check_hashkey_bit_not_set(keys.mod[FLIP_NORMAL], flipx);
        check_hashkey_bit_not_set(keys.mod[FLIP_VERT], flipv);
        check_hashkey_bit_not_set(keys.mod[FLIP_HORZ], fliph);
        check_hashkey_bit_not_set(keys.mod[FLIP_VERT_HORZ], flipvh);
      }
    }

  for (int j = 0; j < FLIP_TOTAL; j++) {
    check_hashkey_code(keys.mod[j], n_rows, n_cols);
  }
}


//########################################################
// hashstore
//########################################################

#define TABLE_SET_KEY(table,index,k)                          \
   table[index].key[0] = k[0], table[index].key[1] = k[1];

#define TABLE_CMP_KEY(table,index,k)                              \
   (table[index].key[0] == k[0] && table[index].key[1] == k[1])

static inline bool hash_store_entry(
    const HashKey& key, s32bit value, s32bit alpha, s32bit beta,
    u32bit nodes, s32bit depth, const Move& best) {
  int index = key.code;

  if(g_trans_table[index].nodes <= nodes){
    TABLE_SET_KEY(g_trans_table, index, key.key);
    g_trans_table[index].nodes = nodes;
    //g_trans_table[index].mask_index = best.mask_index;
    //g_trans_table[index].array_index = best.array_index;
    g_trans_table[index].depth = depth;
    g_trans_table[index].value = value;
    if (value>=beta) g_trans_table[index].type =LOWER;
    else if (value>alpha) g_trans_table[index].type =EXACT;
    else g_trans_table[index].type =UPPER;
    // Only store a result once.
    return true;
  }
  return false;
}


void hashstore(const HashKeys& keys, s32bit value, s32bit alpha, s32bit beta,
               u32bit nodes, s32bit depth, const Move& best) {
  for (int i = 0; i < FLIP_TOTAL; i++) {
    if (hash_store_entry(keys.mod[i], value, alpha, beta, nodes, depth, best)) {
      return;
    }
  }
}


//########################################################
// hashlookup
//########################################################

static inline int hash_lookup_entry(
    const HashKey& key, s32bit *value, s32bit *alpha, s32bit *beta,
    s32bit depth_remaining, Move *force_first) {
  int index = key.code;
  if (TABLE_CMP_KEY(g_trans_table, index, key.key)) {
    /* found matching entry.*/

    /* If nothing else we can use this entry to give us a good move. */
    //force_first->mask_index = g_trans_table[index].mask_index;
    //force_first->array_index = g_trans_table[index].array_index;

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

int hashlookup(const HashKeys& keys, s32bit *value, s32bit *alpha, s32bit *beta,
               s32bit depth_remaining, Move *force_first) {
  for (int j = 0; j < FLIP_TOTAL; j++) {
    int rv = hash_lookup_entry(keys.mod[j], value, alpha, beta,
                           depth_remaining, force_first);
    if (rv != -1) return rv;
  }
  return 0;
}
