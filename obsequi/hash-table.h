#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "move.h"
#include "utils.h"

#define HASH_KEY_SIZE 4

struct HashKey {
  u32bit key[HASH_KEY_SIZE];
  u32bit code;

  void XOR(const HashKey& mod);
  void Print() const;
  // void Validate(const Board& board);
};


void init_hashtable(s32bit num_rows, s32bit num_cols, s32bit board[30][30]);

void check_hash_code_sanity();

void toggle_hash_code(int whos_turn, const Move& move);

void hashstore(s32bit value, s32bit alpha, s32bit beta,
               u32bit nodes, s32bit depth_remaining,
               const Move& best, s32bit player);

int hashlookup(s32bit *value, s32bit *alpha, s32bit *beta,
               s32bit depth_remaining,
               Move *force_first, s32bit player);

void   print_external();

/*
struct HashKey {
  HashKey();
  ~HashKey() {}

  // Prints the value of key_ and code_.
  void Print();

  // Validates that they key_ and code_ are consistent with each other.
  void Validate();

 private:
  u32bit key_[KEYSIZE]; // Bitmap of occupied board positions.
  s32bit code_; // Zobrist value for the given board position.
};
*/
#endif // HASH_TABLE_H
