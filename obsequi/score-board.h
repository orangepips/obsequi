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
  opp_upper_bound = MIN_TWO(upper_bound, upper_bound_real);
  if (curr_lower_bound > opp_upper_bound) {
    // current player wins.
    *score = 5000;
    return true;
  }

  score_board_next_player(
      opponent, &opp_lower_bound, &upper_bound, &upper_bound_real);
  curr_upper_bound = MIN_TWO(upper_bound, upper_bound_real);
  if(opp_lower_bound >= curr_upper_bound) {
    // opponent wins.
    *score = -5000;
    return true;
  }

  //printf("%d -- %d:%d -- %d\n",
  //       curr_lower_bound, curr_upper_bound, opp_lower_bound, opp_upper_bound);
  int curr_avg = (curr_upper_bound + curr_lower_bound) / 2;
  int opp_avg = (opp_upper_bound + opp_lower_bound) / 2;
  if (curr_avg > opp_avg + 2) {
    int avg_len = ((curr_upper_bound - curr_lower_bound) +
                   (opp_upper_bound - opp_lower_bound)) / 2;
    int overlap = (opp_upper_bound + 1) - curr_lower_bound;
    //printf("X %d %d\n", overlap, avg_len);
    *score = 100 - ((100 * overlap)/avg_len);
  } else if (opp_avg > curr_avg + 2) {
    int avg_len = ((curr_upper_bound - curr_lower_bound) +
                   (opp_upper_bound - opp_lower_bound)) / 2;
    int overlap = curr_upper_bound - opp_lower_bound;
    //printf("Y %d %d\n", overlap, avg_len);
    *score = -(100 - ((100 * overlap)/avg_len));
  } else {
    *score = curr_avg - opp_avg;
  }
  //printf("score: %d\n", *score);
  return false;
}

}  // namespace obsequi
#endif  // SCORE_BOARD_H
