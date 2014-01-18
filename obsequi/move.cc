#include "move.h"

#include "bitops.h"
#include "board.h"

#include <assert.h>

#define TWO_STAGE_GENERATION

namespace obsequi {

//=================================================================
// This function generates all the moves in one pass.
//=================================================================
int move_generator(const Board& board, Move movelist[]) {
  int count = 0;
  const u32bit* rows = board.GetBoard();

  for (int i = 0; i < board.GetNumRows(); i++) {
    u32bit curr_row = rows[i+1];

    // m contains a 1 at each position where there is valid move.
    // (except safe moves, since there is no need to play them)
    u32bit prot_rows = rows[i] & rows[i+2];
    u32bit m = ~((curr_row|(curr_row>>1)) | (prot_rows&(prot_rows>>1)));

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
int move_generator_stage1(const Board& board, Move movelist[]) {
  int count = 0;
  const u32bit* rows = board.GetBoard();

  for (int i = 0; i < board.GetNumRows(); i++){
    u32bit curr_row = rows[i+1];
    u32bit prot_rows = rows[i] & rows[i+2];

    // m will contain a 1 at each position that there is a move
    // which is a vulnerable move with no protected squares.
    u32bit m = ~((curr_row|(curr_row>>1)) | (prot_rows|(prot_rows>>1)));

    while (m){
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

int move_generator_stage2(const Board& board, int start, Move movelist[]) {
  int count = start;
  const u32bit* rows = board.GetBoard();

  for (int i = 0; i < board.GetNumRows(); i++){
    u32bit curr_row = rows[i+1];
    u32bit prot_rows = rows[i] & rows[i+2];

    // m will contain a 1 at each position that there is a move
    //   which is a vulnerable move with a protected squares.
    u32bit m = ~((curr_row|(curr_row>>1)) | (~(prot_rows^(prot_rows>>1))) );

    while (m){
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


//********************************************************
// Move sorting function.
//
// This is where the program spends some of the larger
//   portions of its time. Sometime it should be sped up.
//********************************************************

#define NUM_BUCKETS 128

//=================================================================
// This is a stable sort algorithm (uses a bucket sort algorithm).
//=================================================================
void sort_moves(Move movelist[MAX_MOVES], s32bit start, s32bit num_moves) {
  Move bucket[NUM_BUCKETS][MAX_MOVES];
  s32bit buck_val[NUM_BUCKETS];
  s32bit buck_size[NUM_BUCKETS];

  s32bit num_buckets = 0, i, j;

  // place each move in it's proper bucket.
  for (i = start; i < num_moves; i++){
    for (j = 0; j < num_buckets; j++){
      if (movelist[i].info == buck_val[j]){
        bucket[j][buck_size[j]++] = movelist[i];
        break;
      }
    }
    if (j == num_buckets){
      if (j == NUM_BUCKETS) fatal_error(1, "Not enough buckets.\n");
      bucket[j][0] = movelist[i];
      buck_val[j] = movelist[i].info;
      buck_size[j] = 1;
      num_buckets++;
    }
  }

  // remove the moves from their buckets in the proper order.
  {
    s32bit best, index, count = start;

    while (count != num_moves){

      best = buck_val[0];
      index = 0;

      for (i = 1; i < num_buckets; i++)
        if (buck_val[i] > best) index = i, best = buck_val[i];

      // every bucket must have at least one move.
      i = 0;
      do {
        movelist[count++] = bucket[index][i++];
      } while (i < buck_size[index]);

      buck_val[index] = -5000;
    }
  }
}


static inline void score_and_get_first(Board* board, Move movelist[],
                                       int num_moves) {
  int max = -50000;
  int max_index = -1;

  // assert(num_moves > 0);

  for (int i = 0; i < num_moves; i++) {
    movelist[i].info = board->ScoreMove(movelist[i]);
    if (movelist[i].info > max){
      max = movelist[i].info;
      max_index = i;
    }
  }

  // put biggest at front
  if (num_moves > 1) {
    Move tmp_move = movelist[max_index];
    for (int i = max_index; i > 0; i--)
      movelist[i] = movelist[i - 1];
    movelist[0] = tmp_move;
  }
}

bool MoveList::GenerateAllMoves(Board* board) {
  stage_ = 1000000;
  length_ = move_generator(*board, moves_);
  assert(length_);
  score_and_get_first(board, moves_, length_);
  sort_moves(moves_, 1, length_);
  return true;
}

bool MoveList::GenerateNextMoves(Board* board) {
#ifdef TWO_STAGE_GENERATION
  if (stage_ == 0) {
    true_length_ = move_generator_stage1(*board, moves_);
    stage_ = 1;
    if(true_length_ == 0){
      true_length_ = move_generator_stage2(*board, 0, moves_);
      stage_ = 3;
      assert(true_length_);
    }
    score_and_get_first(board, moves_, true_length_);
    length_ = 1;
    return true;
  } else if (stage_ == 1) {
    sort_moves(moves_, 1, true_length_);
    length_ = true_length_;
    stage_ = 2;
    return true;
  } else if (stage_ == 2) {
    length_ = move_generator_stage2(*board, length_, moves_);
    stage_ = 4;
    return true;
  } else if (stage_ == 3) {
    sort_moves(moves_, 1, true_length_);
    length_ = true_length_;
    stage_ = 4;
    return true;
  }
#else
  if (stage_ == 0) {
    length_ = 1;
    true_length_ = move_generator(*board, moves_);
    assert(true_length_);
  
    score_and_get_first(board, moves_, true_length_);
    stage_ = 1;
    return true;
  } else if (stage_ == 1) {
    sort_moves(moves_, 1, true_length_);
    length_ = true_length_;
    stage_ = 2;
    return true;
  }
#endif
  return false;
}

/* Optional sort method.
bool wayToSort(Move i, Move j) { return i.info > j.info; }
std::stable_sort(movelist, movelist+num_moves, wayToSort);
std::__inplace_stable_sort(movelist, movelist+num_moves, wayToSort);
*/

}  // namespace obsequi
