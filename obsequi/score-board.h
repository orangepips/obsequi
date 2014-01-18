#ifndef SCORE_BOARD_H
#define SCORE_BOARD_H

#include "base.h"

namespace obsequi {

class Board;

void score_board_curr_player(const Board& board, int *lower_bound,
                             int *upper_bound, int* upper_bound_real);
void score_board_next_player(const Board& board, int *lower_bound,
                             int *upper_bound, int* upper_bound_real);

static inline bool is_game_over_expensive(
    const Board& curr, const Board& opponent, int* score) {

  int upper_bound, upper_bound_real;
  int curr_lower_bound, opp_upper_bound;
  int curr_upper_bound, opp_lower_bound;

  score_board_curr_player(
      curr, &curr_lower_bound, &upper_bound, &upper_bound_real);
  opp_upper_bound = upper_bound;
  if (curr_lower_bound > opp_upper_bound) {
    // current player wins.
    *score = 5000;
    return true;
  }

  score_board_next_player(
      opponent, &opp_lower_bound, &upper_bound, &upper_bound_real);
  curr_upper_bound = upper_bound;
  if(opp_lower_bound >= curr_upper_bound) {
    // opponent wins.
    *score = -5000;
    return true;
  }

  *score = 0;
  //printf("score: %d\n", *score);
  return false;
}

}  // namespace obsequi
#endif  // SCORE_BOARD_H
