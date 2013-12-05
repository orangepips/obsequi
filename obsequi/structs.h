#ifndef STRUCTS_H
#define STRUCTS_H

#include "utils.h"

//########################################################
// Info we need to describe a move.
//########################################################
typedef struct
{
  s32bit array_index;
  s32bit mask_index;
  s32bit info;
} Move;

#endif
