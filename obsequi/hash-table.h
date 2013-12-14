#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "move.h"
#include "utils.h"

// 64 bits * 2 - Can handle boards with 128 positions
#define HASH_KEY_SIZE 2

// 4 ways we can flip the board: none, vert, horz, and vert/horz.
#define FLIP_TOTAL 4

struct HashKey {
  u64bit key[HASH_KEY_SIZE];
  u32bit code;
};

struct HashKeys {
  void Init(int num_rows, int num_cols, int bit1, int bit2);
  void Print() const;

  void Xor(const HashKeys& move) {
    for (int i = 0; i < FLIP_TOTAL; i++) {
      for (int j = 0; j < HASH_KEY_SIZE; j++) {
        this->mod[i].key[j] ^= move.mod[i].key[j];
      }
      this->mod[i].code ^= move.mod[i].code;
    }
  }

  HashKey mod[FLIP_TOTAL];
};

extern HashKeys g_hashkey;

void init_hashtable(s32bit num_rows, s32bit num_cols, s32bit board[30][30]);

void check_hash_code_sanity();

void hashstore(s32bit value, s32bit alpha, s32bit beta,
               u32bit nodes, s32bit depth_remaining,
               const Move& best, s32bit player);

int hashlookup(s32bit *value, s32bit *alpha, s32bit *beta,
               s32bit depth_remaining,
               Move *force_first, s32bit player);

#endif // HASH_TABLE_H
