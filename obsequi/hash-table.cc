#include "hash-table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//########################################################
// Constants used in conjuction with the transposition table
//   and the zobrist values.
//########################################################

#define FLIP_NORMAL 0
#define FLIP_VERT 1
#define FLIP_HORZ 2
#define FLIP_VERT_HORZ 3

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
static void fill_in_hash_code(HashKey *info, int num_cols, int bit) {
  int r = bit / num_cols;
  int c = bit % num_cols;
  info->code ^= get_zobrist_value(r, c);
  info->key[bit/64] ^= NTH_BIT(bit%64);
}

void HashKeys::Toggle(int bit) {
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

void HashKeys::Init(int num_rowsx, int num_colsx) {
  num_rows = num_rowsx;
  num_cols = num_colsx;
  memset(mod, 0, sizeof(mod));
}

void HashKeys::Print() const {
  for (int i = 0; i < FLIP_TOTAL; i++) {
    printf("Dir: %d, Key: %8lX:%8lX, Code: %8X.\n",
           i, mod[i].key[0], mod[i].key[1], mod[i].code);
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
  mask = hash_size - 1;

  // Initialize trans_table
  table = new HashEntry[hash_size];
  memset(table, 0, sizeof(table[0]) * hash_size);
}

void TranspositionTable::Store(const HashKeys& keys, u8bit depth,
                               u32bit nodes, int value) {
  for (int i = 0; i < HashKeys::FLIP_TOTAL; i++) {
    int index = keys.mod[i].code & mask;
    HashEntry* entry = &table[index];

    if(entry->nodes <= nodes){
      entry->key[0] = keys.mod[i].key[0];
      entry->key[1] = keys.mod[i].key[1];
      entry->nodes = nodes;
      entry->depth = depth;
      entry->value = value;
      // Only store a result once.
      return;
    }
  }
}

bool TranspositionTable::Lookup(const HashKeys& keys, u8bit depth,
                                int *value) {
  for (int i = 0; i < HashKeys::FLIP_TOTAL; i++) {
    int index = keys.mod[i].code & mask;
    HashEntry* entry = &table[index];

    /* found matching entry.*/
    if (entry->key[0] == keys.mod[i].key[0] &&
        entry->key[1] == keys.mod[i].key[1]) {
  
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
