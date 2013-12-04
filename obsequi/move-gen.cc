// The following functions are used to generate all the possible
// moves which could be made, for a certain player, given
// the current board position.

#include "globals.h"
#include "lastbit.h"
#include "structs.h"

// We have proven that there are is no need to play safe moves.
// (unless that is all that is left to play).
#undef PLAY_SAFE_MOVES

//=================================================================
// This function generates all the moves in one pass.
//=================================================================
int
move_generator(int rows, u32bit board[], Move movelist[]) {
  int count = 0;

  for (int i = 0; i < rows; i++) {
    u32bit curr_row = board[i+1];

#ifndef PLAY_SAFE_MOVES
    // m contains a 1 at each position where there is valid move.
    // (except safe moves, since there is no need to play them)
    u32bit prot_rows = board[i] & board[i+2];
    u32bit m = ~((curr_row|(curr_row>>1)) | (prot_rows&(prot_rows>>1)));
#else
    u32bit m = ~(curr_row|(curr_row>>1));
#endif

    while (m) {
      u32bit tmp = (m&-m);  // least sig bit of m
      m ^= tmp;  // remove least sig bit of m.
      movelist[count].mask_index  = lastbit32(tmp);
      movelist[count].array_index = i+1;
      movelist[count].info        = 0;
      count++;
    }
  }

  return count;
}

//=================================================================
// The following functions use a two pass system. This means possibly
//   less cost in evaluating (because if we find a cutoff quickly
//   we will not have to evaluate the second half) and in sorting the
//   moves.
//=================================================================
int
move_generator_stage1(int rows, u32bit board[], Move movelist[]) {
  int count = 0;

  for(int i = 0; i < rows; i++){
    u32bit curr_row = board[i+1];
    u32bit prot_rows = board[i] & board[i+2];
    
    // m will contain a 1 at each position that there is a move
    // which is a vulnerable move with no protected squares.
    u32bit m = ~((curr_row|(curr_row>>1)) | (prot_rows|(prot_rows>>1)));
    
    while(m){
      u32bit tmp = (m&-m); // least sig bit of m
      m ^= tmp;     // remove least sig bit of m.
      movelist[count].mask_index  = lastbit32(tmp);
      movelist[count].array_index = i+1;
      movelist[count].info        = 0;
      count++;
    }
  }

  return count;
}

int
move_generator_stage2(int rows, u32bit board[],
                      int start, Move movelist[]) {
  int count = start;

  for(int i = 0; i < rows; i++){
    u32bit curr_row = board[i+1];
    u32bit prot_rows = board[i] & board[i+2];
    
#ifndef PLAY_SAFE_MOVES
    // m will contain a 1 at each position that there is a move
    //   which is a vulnerable move with a protected squares.
    u32bit m = ~((curr_row|(curr_row>>1)) | (~(prot_rows^(prot_rows>>1))) );
#else
    // m will contain a 1 at each position that there is a move
    //   which is a vulnerable move with a protected squares.
    u32bit m = ((~((curr_row|(curr_row>>1)) | (prot_rows|(prot_rows>>1))))
         ^ (~(curr_row|(curr_row>>1))));
#endif
    
    while(m){
      u32bit tmp = (m&-m); // least sig bit of m
      m ^= tmp;     // remove least sig bit of m.
      movelist[count].mask_index  = lastbit32(tmp);
      movelist[count].array_index = i+1;
      movelist[count].info        = 0;
      count++;
    }
  }

  return count;
}
