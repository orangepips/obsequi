#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "move.h"
#include "utils.h"

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

#endif // HASH_TABLE_H
