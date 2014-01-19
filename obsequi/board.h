// Class to handle the board state.
#ifndef OBSEQUI_BOARD_H
#define OBSEQUI_BOARD_H

#include "base.h"
#include "move.h"
#include "transposition.h"

#include <memory>

namespace obsequi {

class Move;
class PositionalValues;

// Basic board info which we keep track of.
struct BasicInfo {
  int safe;
  int real;
};

class Board {
 public:
  // Constructor actually creates two boards, *this (horizontal) and the
  // opponent (vertical).
  Board(int num_rows, int num_cols);
  ~Board();

  // GetNumCols = GetOpponent()->GetNumRows()
  int GetNumRows() const { return num_rows_; }
  Board* GetOpponent() const { return opponent_; }

  void SetBlock(int row, int col);
  void ApplyMove(const Move& move);
  void UndoMove(const Move& move);
  // Sadly not const since we modify internal state for performance reasons.
  int ScoreMove(const Move& move);

  // Note that the array returned is always a length of 32.
  const u32bit* GetBoard() const { return board_; }
  const BasicInfo& GetInfo() const { return info_totals; }
  int GetEmptySquares() const { return shared->empty_squares; }
  const HashKeys& GetHashKeys() const { return shared->hashkey; }

  bool IsGameOver(int* score) const;

  void Print() const;
  void PrintInfo() const;
  void PrintBitboard() const;

 private:
  // Constructor for vertical.
  Board(int num_rows, int num_cols, Board* opponent);
  void Initialize(int num_rows, int num_cols, Board* opponent);
  void InitInfo();

  const HashKeys& GetHashKeys(const Move& move) const {
    return move_hash_keys_[move.array_index][move.mask_index];
  }

  void ToggleMove(const Move& move);
  void UpdateSafe(int row);
  void UpdateReal(int row);

  struct BoardShared {
    int empty_squares;
    HashKeys hashkey;
  };

  // bitmap of the current board state.
  u32bit board_[32];

  // You can get the number of cols by looking at the opponent->num_rows.
  const int num_rows_;
  const bool is_horizontal;

  // Basic safe move/real move stats.
  BasicInfo info[32];
  BasicInfo info_totals;

  // this* and opponent_ should always be kept consistent.
  std::unique_ptr<Board> owner_opponent_;
  Board* opponent_;

  std::unique_ptr<PositionalValues> position;

  std::unique_ptr<BoardShared> owner_shared_;
  BoardShared* shared;

  // HashKeys associated with every move.
  HashKeys move_hash_keys_[32][32];

 private:
  // Disallow copy and assign.
  Board(const Board&);
  void operator=(const Board&);
};

inline bool Board::IsGameOver(int* score) const {
  if(this->info_totals.safe > opponent_->info_totals.real){
    // current player wins.
    *score = 5000;
    return true;
  }

  if(opponent_->info_totals.safe >= this->info_totals.real){
    // opponent wins.
    *score = -5000;
    return true;
  }
  return false;
}

}  // namespace obsequi
#endif  // OBSEQUI_BOARD_H
