// Functions for generating the moves that can be made in a given
// board position.
#ifndef MOVE_GEN_H
#define MOVE_GEN_H

#include "consts.h"
#include "board.h"

struct Move {
  int array_index;
  int mask_index;
  int info;
};

// Generate all the moves that can be made.
// Places x moves into movelist and returns x.
int move_generator(const Board& board,
                   Move movelist[MAX_MOVES]);

// Generate moves in two parts. (Union of these two stages should equal above.)
// - stage1 is all the .... moves.
// - stage2 is all the .... moves.
// Places x moves into movelist and returns x.
int move_generator_stage1(const Board& board,
                          Move movelist[MAX_MOVES]);
int move_generator_stage2(const Board& board,
                          int start, Move movelist[MAX_MOVES]);

// Score all the moves (and move the best to the front of the list).
void score_and_get_first(Board* board, Move movelist[MAXMOVES], 
                         int num_moves, Move first);

// Sort moves into a decending order. (Stable sort.)
void sort_moves(Move movelist[MAXMOVES], int start, int num_moves);


class MoveList {
 public:
  MoveList() : length_(0), stage_(0) {}
  ~MoveList() {}

  // Generate all possible moves and sort them.
  bool GenerateAllMoves(Board* board) {
    stage_ = 1000000;
    length_ = move_generator(*board, moves_);
    CHECK(length_, "No moves.");
    Move first;
    first.array_index = -1;
    score_and_get_first(board, moves_, length_, first);
    sort_moves(moves_, 1, length_);
    return true;
  }

  // Generates moves in stages. (First is a hint of a likely good move.)
  // First call should always return just 1. (Best move).
  // Each subsequent call will generate more until false is returned.
  bool GenerateNextMoves(Board* board, Move first) {
#ifdef TWO_STAGE_GENERATION
    if (stage_ == 0) {
      true_length_ = move_generator_stage1(*board, moves_);
      stage_ = 1;
      if(true_length_ == 0){
        true_length_ = move_generator_stage2(*board, 0, moves_);
        stage_ = 3;
        CHECK(true_length_, "No moves.");
      }
      score_and_get_first(board, moves_, true_length_, first);
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
      CHECK(true_length_, "No moves.");
    
      score_and_get_first(board, moves_, true_length_, first);
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

  Move operator[](int index) {
    return moves_[index];
  }
    
  int length() {
    return length_;
  }

 private:
  Move moves_[MAX_MOVES];
  int length_;
  int stage_;
  int true_length_;
};

#endif //ifndef MOVE_GEN_H
