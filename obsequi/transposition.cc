#include "transposition.h"

#include <cstring>
#include <stdlib.h>

//########################################################
// Constants used in conjuction with the transposition table
//   and the zobrist values.
//########################################################

namespace obsequi {

// ##################################################################
// Zobrist functionality.
// ##################################################################
u32bit zobrist[32 * 32];

void __attribute__ ((constructor)) init_zobrist() {
  srandom(1);
  for (int i = 0; i < 32*32; i++) {
    zobrist[i] = random();
  }
}

u32bit get_zobrist_value(int row, int col) {
  return zobrist[(row+1) * 32 + (col+1)];
}

// ##################################################################
// HashKeys class.
// ##################################################################
#define FLIP_NORMAL 0
#define FLIP_VERT 1
#define FLIP_HORZ 2
#define FLIP_VERT_HORZ 3

static inline void fill_in_hash_code(HashKey* info, int num_cols, int bit) {
  int r = bit / num_cols;
  int c = bit % num_cols;
  info->code ^= get_zobrist_value(r, c);
  info->key[bit/64] ^= NTH_BIT(bit%64);
}

void HashKeys::Toggle(int bit) {
  int row = bit / num_cols_;
  int col = bit % num_cols_;

  fill_in_hash_code(&keys_[FLIP_NORMAL], num_cols_, bit);
  fill_in_hash_code(&keys_[FLIP_VERT], num_cols_,
      (row * num_cols_) + (num_cols_ - col - 1));
  fill_in_hash_code(&keys_[FLIP_HORZ], num_cols_,
      ((num_rows_ - row - 1) * num_cols_) + col);
  fill_in_hash_code(&keys_[FLIP_VERT_HORZ], num_cols_,
      ( ((num_rows_ - row - 1) * num_cols_) + (num_cols_ - col - 1) ));
}

void HashKeys::Init(int num_rows, int num_cols) {
  num_rows_ = num_rows;
  num_cols_ = num_cols;
  memset(keys_, 0, sizeof(keys_));
}

void HashKeys::Print() const {
  for (int i = 0; i < FLIP_TOTAL; i++) {
    printf("Dir: %d, Key: %8lX:%8lX, Code: %8X.\n",
           i, keys_[i].key[0], keys_[i].key[1], keys_[i].code);
  }
}

// ##################################################################
// print_hashentry
// ##################################################################
void print_hashentry(s32bit index) {
  //Hash_Entry entry = g_trans_table[index];

  printf("Hash entry: %d.\n", index);
  /*
  printf(" Key:%8lX:%8lX, n:%u, d:%d, w:%d"
         ", v:%d, t:%d, int:%d,%d.\n",
         entry.key[0], entry.key[1],
         entry.nodes, // entry.depth, 0, //entry.whos_turn,
         entry.value, // entry.type,
         0, 0);//entry.mask_index, entry.array_index);
         */
}


//########################################################
// check_hash_code_sanity
//########################################################

/*
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
*/

extern void
check_hash_code_sanity(const HashKeys& keys) {
  /*
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
  */
}


// Transposition table.
TranspositionTable* trans_table;

TranspositionTable::TranspositionTable(int bits) {
  int hash_size = (1 << bits);
  bits_ = bits;
  mask_ = hash_size - 1;

  // Initialize trans_table
  table_.reset(new HashEntry[hash_size]);
  memset(table_.get(), 0, sizeof(table_[0]) * hash_size);
}

int TranspositionTable::GetBits() const {
  return bits_;
}

void TranspositionTable::Store(const HashKeys& hash_keys, u8bit depth,
                               u32bit nodes, int value) {
  const HashKey* keys = hash_keys.GetKeys();
  for (int i = 0; i < HashKeys::FLIP_TOTAL; i++) {
    int index = keys[i].code & mask_;
    HashEntry* entry = &table_[index];

    // TODO: We should try finding the minimum and replacing that one, instead
    // of just the first one.
    if(entry->nodes <= nodes){
      entry->key[0] = keys[i].key[0];
      entry->key[1] = keys[i].key[1];
      entry->nodes = nodes;
      entry->depth = depth;
      entry->value = value;
      // Only store a result once.
      return;
    }
  }
}

bool TranspositionTable::Lookup(const HashKeys& hash_keys, u8bit depth,
                                int* value) {
  const HashKey* keys = hash_keys.GetKeys();
  for (int i = 0; i < HashKeys::FLIP_TOTAL; i++) {
    int index = keys[i].code & mask_;
    HashEntry* entry = &table_[index];

    /* found matching entry.*/
    if (entry->key[0] == keys[i].key[0] && entry->key[1] == keys[i].key[1]) {
  
      /* use value if depth >= than the depth remaining in our search. */
      if(entry->depth >= depth) {
        *value=entry->value;
        return 1;
      }
    }
  }
  return 0;
}

}  // namespace obsequi
