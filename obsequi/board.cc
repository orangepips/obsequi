#include "board.h"

#include "bitops.h"
#include "board-ops.h"
#include "move.h"
#include "positional-values.h"

namespace obsequi {

// Initialize Horizontal board.
Board::Board(int num_rows, int num_cols)
    : num_rows_(num_rows), is_horizontal(true) {
  owner_opponent_.reset(new Board(num_cols, num_rows, this));
  this->Initialize(num_rows, num_cols, owner_opponent_.get());

  owner_shared_.reset(new BoardShared());
  owner_opponent_->shared = shared = owner_shared_.get();
  shared->empty_squares = num_rows * num_cols;
  shared->hashkey.Init(num_rows, num_cols);
}

Board::~Board() {}

// Initialize Vertical board.
Board::Board(int num_rows, int num_cols, Board* opponent)
    : num_rows_(num_rows), is_horizontal(false) {
  this->Initialize(num_rows, num_cols, opponent);
}

void Board::Initialize(int num_rows, int num_cols, Board* opponent) {
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
    this->board_[i] = ALL_BITS_32;
    this->info[i].real = 0;
    this->info[i].safe = 0;
  }
  this->info_totals.real = 0;
  this->info_totals.safe = 0;

  // Clear positions where there isn't a piece.
  for(int i = 0; i < num_rows; i++) {
    for(int j = 0; j < num_cols; j++) {
      this->board_[i+1] &= ~(NTH_BIT(j+1));
    }
  }

  this->InitInfo();
  this->position.reset(new PositionalValues(num_rows, num_cols));

  // Initialize g_keyinfo
  for (int i = 0; i < num_rows; i++) {
    for (int j = 0; j < num_cols; j++) {
      if (is_horizontal) {
        move_hash_keys_[i+1][j+1].Init(num_rows, num_cols);
        move_hash_keys_[i+1][j+1].Toggle((i*num_cols)+j);
        move_hash_keys_[i+1][j+1].Toggle((i*num_cols)+j+1);
      } else {
        // Vertical player, hash keys reflect horizontal view
        move_hash_keys_[i+1][j+1].Init(num_cols, num_rows);
        move_hash_keys_[i+1][j+1].Toggle((j*num_rows)+i);
        move_hash_keys_[i+1][j+1].Toggle(((j+1)*num_rows)+i);
      }
    }
  }
}

inline void Board::UpdateSafe(int row) {
  int count = count_safe(board_, row);

  info_totals.safe += count - info[row].safe;
  info[row].safe = count;
}

inline void Board::UpdateReal(int row) {
  int count = count_real(board_, row);

  info_totals.real += count - info[row].real;
  info[row].real = count;
}

void Board::SetBlock(int row, int col) {
  board_[row+1] |= NTH_BIT(col+1);
  opponent_->board_[col+1] |= NTH_BIT(row+1);
  InitInfo();
  opponent_->InitInfo();
  shared->hashkey.Toggle((row*opponent_->num_rows_)+col);
  shared->empty_squares--;
}

inline void Board::ToggleMove(const Move& move) {
  Board* opp = opponent_;

  int row = move.array_index;
  int col = move.mask_index;

  this->board_[row] ^= (3<<col);
  opp->board_[col] ^= (1<<row);
  opp->board_[col+1] ^= (1<<row);

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

int Board::ScoreMove(const Move& move) {
  int row = move.array_index;
  int col = move.mask_index;

  u32bit* opp_board = GetOpponent()->board_;
  board_[row] ^= (3<<col);
  opp_board[col] ^= (1<<row);
  opp_board[col+1] ^= (1<<row);

  int safe_created = 0;
  int score = 0;
  BasicInfo* opp_info = GetOpponent()->info;

  // update real moves
  score += count_real(board_, row) - info[row].real;

  score -= count_real(opp_board, col) - opp_info[col].real;
  score -= count_real(opp_board, col + 1) - opp_info[col+1].real;

  // update safe moves
  if(row - 1 != 0)
    safe_created += count_safe(board_, row - 1) - info[row-1].safe;
  score += count_safe(board_, row) - info[row].safe;
  if(row != GetNumRows())
    safe_created += count_safe(board_, row+1) - info[row+1].safe;

  if(col - 1 != 0)
    score -= count_safe(opp_board, col - 1) - opp_info[col-1].safe;
  if(col + 1 != GetOpponent()->GetNumRows())
    score -= count_safe(opp_board, col + 2) - opp_info[col+2].safe;

  score += safe_created;
  /* The following does seem to slightly improve move ordering.
  if (safe_created > 0) {  // Creating safe moves is really good.
    score += 2;
  } else {
    u32bit mask = (3 << col);
    if (((mask & board_[row+1]) == mask && (mask & ~board_[row-1]) == mask) ||
        ((mask & board_[row-1]) == mask && (mask & ~board_[row+1]) == mask)) {
      // Playing in a position where a safe move could have been created is
      // just silly.
      score -= 20;
    }
  }
  */

  // Reset the board_ to how we found it.
  // Sadly this makes it so this method isn't const.
  board_[row] ^= (3<<col);
  opp_board[col] ^= (1<<row);
  opp_board[col+1] ^= (1<<row);

  score *= 128;
  score += position->GetValue(row, col);

  return score;
}

void Board::ApplyMove(const Move& move) {
  shared->empty_squares -= 2;
  ToggleMove(move);
  shared->hashkey.Xor(GetHashKeys(move));
}

void Board::UndoMove(const Move& move) {
  shared->empty_squares += 2;
  ToggleMove(move);
  shared->hashkey.Xor(GetHashKeys(move));
}

void Board::InitInfo() {
  this->info_totals.safe = 0;
  this->info_totals.real = 0;
  for(int i = 0; i < num_rows_; i++){
    this->info_totals.safe += count_safe(this->board_, i+1);
    this->info_totals.real += count_real(this->board_, i+1);
    this->info[i+1].safe = count_safe(this->board_, i+1);
    this->info[i+1].real = count_real(this->board_, i+1);
  }
}

void Board::Print() const {
  if (!is_horizontal) {
    GetOpponent()->Print();
    return;
  }
  for(int i = 0; i < num_rows_; i++){
    for(int j = 0; j < opponent_->num_rows_; j++){
      if(board_[i+1] & NTH_BIT(j+1))
        printf(" #");
      else
        printf(" 0");
    }
    printf("\n");
  }
}

void Board::PrintInfo() const {
  if (!is_horizontal) {
    GetOpponent()->PrintInfo();
    return;
  }

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
  if (!is_horizontal) {
    GetOpponent()->PrintBitboard();
    return;
  }

  for(int i = 0; i < this->num_rows_ + 2; i++)
    printf("Ox%X\n", this->board_[i]);
}

//========================================================
// This function compares the Horizontal board_ info to
//  the vertical board info.
//========================================================
/*
extern void
check_board_sanity()
{
  int i, j;
  int count;

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
}  // namespace obsequi
