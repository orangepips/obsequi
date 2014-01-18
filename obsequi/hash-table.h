#ifndef OBSEQUI_HASH_TABLE_H
#define OBSEQUI_HASH_TABLE_H

#include "base.h"

namespace obsequi {

// 64 bits * 2 - Can handle boards with 128 positions
const int KEY_SIZE = 2;

struct HashKey {
  u64bit key[KEY_SIZE];
  u32bit code;
};

class HashKeys {
 public:
  void Init(int num_rows, int num_cols);
  void Toggle(int bit);

  void Print() const;

  void Xor(const HashKeys& move) {
    for (int i = 0; i < FLIP_TOTAL; i++) {
      for (int j = 0; j < KEY_SIZE; j++) {
        this->mod[i].key[j] ^= move.mod[i].key[j];
      }
      this->mod[i].code ^= move.mod[i].code;
    }
  }

  // 4 ways we can flip the board: none, vert, horz, and vert/horz.
  static const int FLIP_TOTAL = 4;
  HashKey mod[FLIP_TOTAL];

 private:
  int num_rows;
  int num_cols;
};

struct HashEntry {
  u64bit key[KEY_SIZE];

  // NOTE: performance seems to be very sensitive on data size.

  // if real num of nodes exceeds ULONG_MAX set to ULONG_MAX.
  //   or maybe we could just shift the bits (larger granularity).
  u32bit nodes;

  // value of node determined with a search to `depth`.
  s16bit value;

  // depth of the search when this value was determined.
  u8bit depth;
};

class TranspositionTable {
 public:
  TranspositionTable(int bits);

  void Store(const HashKeys& keys, u8bit depth_remaining, u32bit nodes,
             int value);

  bool Lookup(const HashKeys& keys, u8bit depth_remaining, int *value);

  // void Stats();

 private:
  u32bit mask;
  HashEntry* table;
};

// TODO: Get rid of this global variable.
extern TranspositionTable* trans_table;

// TODO: Get this to work and move it into the HashKeys class.
void check_hash_code_sanity(const HashKeys& keys);

}  // namespace obsequi
#endif  // OBSEQUI_HASH_TABLE_H
