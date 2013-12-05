// Class to handle the board state.
#ifndef BOARD_H
#define BOARD_H

#include "board-ops.h"
#include "countmoves.h"

#include "structs.h"

// Basic board info which we keep track of.
struct BasicInfo {
  int safe;
  int real;
};

class Board {
 public:
  // Constructor actually creates two boards, this and the opponent.
  Board(int num_rows, int num_cols);
  ~Board() {}

  Board* GetOpponent() const { return opponent_; }
  int GetNumRows() const { return num_rows_; }

  void SetBlock(int row, int col);

  void ToggleMove(const Move& move);

  void Print() const;
  void PrintInfo() const;
  void PrintBitboard() const;

  bool IsGameOver(int* score) const {
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

 private:
  void UpdateSafe(int row) {
    int count = count_safe(board, row);

    info_totals.safe += count - info[row].safe;
    info[row].safe = count;
  }

  void UpdateReal(int row) {
    int count = count_real(board, row);
  
    info_totals.real += count - info[row].real;
    info[row].real = count;
  }

 private:
  Board(int num_rows, int num_cols, Board* opponent);
  void Initialize(int num_rows, int num_cols, Board* opponent);
  void InitInfo();

  // You can get the number of cols by looking at the opponent->num_rows.
  const int num_rows_;

  // this and the opponent should always be kept consistent.
  Board* opponent_;

 public:
  // TODO(nathan): need to migrate these all to private.
  u32bit board[32];
  BasicInfo info[32];
  BasicInfo info_totals;

 private:
};

#endif  // BOARD_H
