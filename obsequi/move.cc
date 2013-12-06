// The following functions are used to generate all the possible
// moves which could be made, for a certain player, given
// the current board position.
#include "move.h"

#include "globals.h"
#include "lastbit.h"
#include "positional-values.h"

// We have proven that there are is no need to play safe moves.
// (unless that is all that is left to play).
#undef PLAY_SAFE_MOVES

//=================================================================
// This function generates all the moves in one pass.
//=================================================================
int
move_generator(const Board& board, Move movelist[]) {
  int count = 0;
  const u32bit* rows = board.board;

  for (int i = 0; i < board.GetNumRows(); i++) {
    u32bit curr_row = rows[i+1];

#ifndef PLAY_SAFE_MOVES
    // m contains a 1 at each position where there is valid move.
    // (except safe moves, since there is no need to play them)
    u32bit prot_rows = rows[i] & rows[i+2];
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
move_generator_stage1(const Board& board, Move movelist[]) {
  int count = 0;
  const u32bit* rows = board.board;

  for(int i = 0; i < board.GetNumRows(); i++){
    u32bit curr_row = rows[i+1];
    u32bit prot_rows = rows[i] & rows[i+2];

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
move_generator_stage2(const Board& board, int start, Move movelist[]) {
  int count = start;
  const u32bit* rows = board.board;

  for(int i = 0; i < board.GetNumRows(); i++){
    u32bit curr_row = rows[i+1];
    u32bit prot_rows = rows[i] & rows[i+2];

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
extern void
sort_moves(Move movelist[MAXMOVES], s32bit start, s32bit num_moves)
{
  Move    bucket[NUM_BUCKETS][MAXMOVES];
  s32bit  buck_val[NUM_BUCKETS];
  s32bit  buck_size[NUM_BUCKETS];

  s32bit num_buckets = 0, i, j;

  // place each move in it's proper bucket.
  for(i = start; i < num_moves; i++){
    for(j = 0; j < num_buckets; j++){
      if(movelist[i].info == buck_val[j]){
        bucket[j][buck_size[j]++] = movelist[i];
        break;
      }
    }
    if(j == num_buckets){
      if(j == NUM_BUCKETS) fatal_error(1, "Not enough buckets.\n");
      bucket[j][0] = movelist[i];
      buck_val[j] = movelist[i].info;
      buck_size[j] = 1;
      num_buckets++;
    }
  }

  // remove the moves from their buckets in the proper order.
  {
    s32bit best, index, count = start;

    while(count != num_moves){

      best = buck_val[0];
      index = 0;

      for(i = 1; i < num_buckets; i++)
        if(buck_val[i] > best) index = i, best = buck_val[i];

      // every bucket must have at least one move.
      i = 0;
      do {
        movelist[count++] = bucket[index][i++];
      } while(i < buck_size[index]);

      buck_val[index] = -5000;
    }
  }
}


//#define DEBUG_SCORE_MOVE

static inline s32bit
score_move(Board* boardx, Move move)
{
  u32bit* board = boardx->board;
  u32bit* opp_board = boardx->GetOpponent()->board;

  int row = move.array_index;
  int col = move.mask_index;

  board[row]   ^= (3<<col);
  opp_board[col]   ^= (1<<row);
  opp_board[col+1] ^= (1<<row);

  Board* horz = boardx;
  Board* vert = boardx->GetOpponent();

  // update real moves
  int score = count_real(board, row) - horz->info[row].real;

  score -= count_real(opp_board, col) - vert->info[col].real;
  score -= count_real(opp_board, col + 1) - vert->info[col+1].real;

  // update safe moves
  if(row - 1 != 0)
    score += count_safe(board, row - 1) - horz->info[row-1].safe;
  score += count_safe(board, row) - horz->info[row].safe;
  if(row != horz->GetNumRows())
    score += count_safe(board, row+1) - horz->info[row+1].safe;

  if(col - 1 != 0)
    score -= count_safe(opp_board, col - 1) - vert->info[col-1].safe;
  if(col + 1 != vert->GetNumRows())
    score -= count_safe(opp_board, col + 2) - vert->info[col+2].safe;

  board[row]   ^= (3<<col);
  opp_board[col]   ^= (1<<row);
  opp_board[col+1] ^= (1<<row);

  score *= 128;
  score += boardx->position->GetValue(row, col);

  return score;
}

extern void
score_and_get_first(Board* board, Move movelist[MAXMOVES], s32bit num_moves,
                    Move first)
{
  s32bit i, max = -50000, max_index = -1;

  //========================================================
  // Give the move from the hashtable a large sorting value.
  //========================================================
  /*
  {
    int z = 0;
    for (z = 0; z < num_moves; z++)
      printf("> %d %d\n", movelist[z].array_index, movelist[z].mask_index);
  }
  */
  if(first.array_index != -1){
    for(i = 0; i < num_moves; i++){
      if(first.array_index == movelist[i].array_index
         && first.mask_index == movelist[i].mask_index){
        movelist[i].info = 450000;
        max_index = i;
      } else {
        movelist[i].info = score_move(board, movelist[i]);
      }
    }
  }

  else {
    for(i = 0; i < num_moves; i++){
      movelist[i].info = score_move(board, movelist[i]);
      if(movelist[i].info > max){
        max = movelist[i].info;
        max_index = i;
      }
    }
  }

  if(max_index == -1) fatal_error(1, "No maximum\n");

  // put biggest at front
  if(num_moves > 1) {
    Move tmp_move = movelist[max_index];
    for(i = max_index; i > 0; i--)
      movelist[i] = movelist[i - 1];
    movelist[0] = tmp_move;
  }
}
