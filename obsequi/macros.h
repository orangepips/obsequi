#ifndef MACROS_H
#define MACROS_H

#include "globals.h"
#include "countbits.h"
#include "countmoves.h"
#include "lastbit.h"


#define HASHKEY_UPDATE(key,index) key[index/32] ^= NTH_BIT(index%32)


#define toggle_hash_code(info)  \
                                                              \
HASHKEY_UPDATE(g_norm_hashkey.key, info.norm.bit1_index);     \
HASHKEY_UPDATE(g_norm_hashkey.key, info.norm.bit2_index);     \
g_norm_hashkey.code ^= info.norm.hash_code;                   \
                                                              \
HASHKEY_UPDATE(g_flipV_hashkey.key, info.flipV.bit1_index);   \
HASHKEY_UPDATE(g_flipV_hashkey.key, info.flipV.bit2_index);   \
g_flipV_hashkey.code ^= info.flipV.hash_code;                 \
                                                              \
HASHKEY_UPDATE(g_flipH_hashkey.key, info.flipH.bit1_index);   \
HASHKEY_UPDATE(g_flipH_hashkey.key, info.flipH.bit2_index);   \
g_flipH_hashkey.code ^= info.flipH.hash_code;                 \
                                                              \
HASHKEY_UPDATE(g_flipVH_hashkey.key, info.flipVH.bit1_index); \
HASHKEY_UPDATE(g_flipVH_hashkey.key, info.flipVH.bit2_index); \
g_flipVH_hashkey.code ^= info.flipVH.hash_code;


#endif //MACROS_H
