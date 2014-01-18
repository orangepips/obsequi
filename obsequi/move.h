// Handle generation of possible moves for a given board position

#ifndef OBSEQUI_MOVE_H
#define OBSEQUI_MOVE_H

#include "base.h"

namespace obsequi {

class Board;

struct Move {
  int array_index;
  int mask_index;
  int info;
};

class MoveList {
 public:
  MoveList() : curr_(-1), length_(0), stage_(0) {}

  // Possibly generate moves in stages (will score and sort each stage as it
  // is generated.) Return nullptr when there are no more moves.
  const Move* GetNext(Board* board);

  // Current position (index) of GetNext.
  int Index() { return curr_; }


  // Generate all remaining moves.
  bool GenerateAllMoves(Board* board);

  // The current number of moves that have been generated.
  int Size() const { return length_; }

  const Move& operator[](int index) const { return moves_[index]; }
  Move& operator[](int index) { return moves_[index]; }

 private:
  // Generates moves in stages. (First is a hint of a likely good move.)
  // First call should always return just 1. (Best move).
  // Each subsequent call will generate more until false is returned.
  bool GenerateNextMoves(Board* board);

  Move moves_[MAX_MOVES];
  int curr_;
  int length_;
  int stage_;
  int true_length_;
};

inline const Move* MoveList::GetNext(Board* board) {
  curr_++;
  while (curr_ >= length_) {
    if (!GenerateNextMoves(board)) {
      return nullptr;
    }
  }
  return &moves_[curr_];
}


// Generate all the moves that can be made.
// Returns the number of moves generated.
int move_generator(const Board& board, Move movelist[MAX_MOVES]);

// Generate moves in two parts. (Union of these two stages should equal above.)
// - stage1 is all the .... moves.
// - stage2 is all the .... moves.
// Returns the number of moves generated.
int move_generator_stage1(const Board& board,
                          Move movelist[MAX_MOVES]);
int move_generator_stage2(const Board& board,
                          int start, Move movelist[MAX_MOVES]);

// Score all the moves (and move the best to the front of the list).
// void score_and_get_first(Board* board, Move movelist[MAX_MOVES], 
//                          int num_moves, const Move& first);

// Sort moves into a decending order. (Stable sort.)
// void sort_moves(Move movelist[MAX_MOVES], int start, int num_moves);

}  // namespace obsequi
#endif  // OBSEQUI_MOVE_H
