#ifndef OBSEQUI_TRANSPOSITION_H
#define OBSEQUI_TRANSPOSITION_H

#include "base.h"

#include <memory>

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

  void Xor(const HashKeys& hash_key) {
    for (int i = 0; i < FLIP_TOTAL; i++) {
      for (int j = 0; j < KEY_SIZE; j++) {
        this->keys_[i].key[j] ^= hash_key.keys_[i].key[j];
      }
      this->keys_[i].code ^= hash_key.keys_[i].code;
    }
  }

  const HashKey* GetKeys() const { return keys_; }

  void Print() const;

  // 4 ways we can flip the board: none, vert, horz, and vert/horz.
  static const int FLIP_TOTAL = 4;

 private:
  int num_rows_;
  int num_cols_;
  HashKey keys_[FLIP_TOTAL];
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

  void Store(const HashKeys& keys, u8bit depth, u32bit nodes, int value);
  bool Lookup(const HashKeys& keys, u8bit depth, int *value);

  int GetBits() const;
  // void Stats();

 private:
  u32bit mask_;
  int bits_;
  std::unique_ptr<HashEntry[]> table_;
};

// TODO: Get rid of this global variable.
extern TranspositionTable* trans_table;

// TODO: Get this to work and move it into the HashKeys class.
void check_hash_code_sanity(const HashKeys& keys);

}  // namespace obsequi
#endif  // OBSEQUI_TRANSPOSITION_H
