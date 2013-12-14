#include "board.h"

#include "move.h"
#include "consts.h"
#include "positional-values.h"

// Initialize Horizontal board.
Board::Board(int num_rows, int num_cols)
    : num_rows_(num_rows) {
  Board* opp = new Board(num_cols, num_rows, this);
  this->Initialize(num_rows, num_cols, opp, true /* is_horizontal */);
  opp->shared = shared = new BoardShared();
}

// Initialize Vertical board.
Board::Board(int num_rows, int num_cols, Board* opponent)
    : num_rows_(num_rows) {
  this->Initialize(num_rows, num_cols, opponent, false /* !is_horizontal */);
}

void Board::Initialize(int num_rows, int num_cols, Board* opponent,
    bool is_horizontal) {
  this->opponent_ = opponent;

  if (num_rows < 1 || num_rows > MAX_ROWS-2 ||
      num_cols < 1 || num_cols > MAX_ROWS-2) {
    fatal_error(1, "Invalid board size %dX%d.\n", num_rows, num_cols);
  }

  // Hashtable uses a 128 bit key.
  if (num_rows * num_cols >= 128) {
    fatal_error(1, "Invalid board size %dX%d.\n", num_rows, num_cols);
  }

  // Set all the bits to 1.
  for(int i = 0; i < MAX_ROWS; i++){
    this->board[i] = ALL_BITS;
    this->info[i].real = 0;
    this->info[i].safe = 0;
  }
  this->info_totals.real = 0;
  this->info_totals.safe = 0;

  // Clear positions where there isn't a piece.
  for(int i = 0; i < num_rows; i++) {
    for(int j = 0; j < num_cols; j++) {
      this->board[i+1] &= ~(NTH_BIT(j+1));
    }
  }

  this->InitInfo();
  this->position = new PositionalValues(num_rows, num_cols);

  // Initialize g_keyinfo
  for (int i = 0; i < num_rows; i++) {
    for (int j = 0; j < num_cols; j++) {
      if (is_horizontal) {
        move_hash_keys_[i+1][j+1].
            Init(num_rows, num_cols, (i*num_cols)+j, (i*num_cols)+j+1);
      } else {
        move_hash_keys_[i+1][j+1].
            Init(num_cols, num_rows, (j*num_rows)+i, ((j+1)*num_rows)+i);
      }
    }
  }
}

void Board::SetBlock(int row, int col) {
  this->board[row+1] |= NTH_BIT(col+1);
  this->opponent_->board[col+1] |= NTH_BIT(row+1);
  shared->hashkey.Toggle(num_rows_, opponent_->num_rows_,
      (row*opponent_->num_rows_)+col);
}

void Board::InitInfo() {
  this->info_totals.safe = 0;
  this->info_totals.real = 0;
  for(int i = 0; i < num_rows_; i++){
    this->info_totals.safe += count_safe(this->board, i+1);
    this->info_totals.real += count_real(this->board, i+1);
    this->info[i+1].safe = count_safe(this->board, i+1);
    this->info[i+1].real = count_real(this->board, i+1);
  }
}

void Board::Print() const {
  for(int i = 0; i < num_rows_; i++){
    for(int j = 0; j < opponent_->num_rows_; j++){
      if(board[i+1] & NTH_BIT(j+1))
        printf(" #");
      else
        printf(" 0");
    }
    printf("\n");
  }
}

void Board::PrintInfo() const {
  const Board& curr = *this;
  const Board& opp = *opponent_;

  char    str[32][80], null_str[1] = "";

  sprintf(str[1], "Number of rows    = %d", curr.num_rows_);
  sprintf(str[2], "Number of columns = %d", opp.num_rows_);

  printf("%7s %15s %15s\n",
         null_str, "Horizontal", "Vertical");
  printf("%7s %7s %7s %7s %7s\n",
         null_str, "Real", "Safe", "Real", "Safe");

  int max_dim = MAX_TWO(curr.num_rows_, opp.num_rows_);
  for(int i = 0; i < max_dim; i++){
    printf("%6d) %7d %7d %7d %7d  %s\n", i + 1,
           curr.info[i+1].real,
           curr.info[i+1].safe,
           opp.info[i+1].real,
           opp.info[i+1].safe,
           (i < 2) ? str[i+1] : null_str);
  }

  printf("Totals: %7d %7d %7d %7d\n",
         curr.info_totals.real,
         curr.info_totals.safe,
         opp.info_totals.real,
         opp.info_totals.safe);
}

void Board::PrintBitboard() const {
  for(int i = 0; i < this->num_rows_ + 2; i++)
    printf("Ox%X\n", this->board[i]);
}

void Board::ToggleMove(const Move& move) {
  Board* opp = opponent_;

  int row = move.array_index;
  int col = move.mask_index;

  this->board[row]   ^= (3<<col);
  opp->board[col]   ^= (1<<row);
  opp->board[col+1] ^= (1<<row);

  // update safe moves
  if(row - 1 != 0) this->UpdateSafe(row-1);
  this->UpdateSafe(row);
  if(row != this->num_rows_) this->UpdateSafe(row+1);

  if(col - 1 != 0) opp->UpdateSafe(col-1);
  if(col + 1 != opp->num_rows_) opp->UpdateSafe(col+2);

  // update real moves
  this->UpdateReal(row);

  opp->UpdateReal(col);
  opp->UpdateReal(col+1);
}

//========================================================
// This function compares the Horizontal board info to
//  the vertical board info.
//========================================================
/*
extern void
check_board_sanity()
{
  s32bit i, j;
  s32bit count;

  for(j = 0; j < g_board_size[HORIZONTAL] + 2; j++)
    for(i = 0; i < g_board_size[VERTICAL] + 2; i++){
      count = 0;

      if(g_board[VERTICAL][i] & NTH_BIT(j)) count++;
      if(g_board[HORIZONTAL][j] & NTH_BIT(i)) count++;

      if(count == 1){
        print_board(VERTICAL);
        print_board(HORIZONTAL);

        printf("%d %d - %d.\n", j, i, count);

        fatal_error(1, "Board is inconsistent.\n");
      }
    }
}
*/
