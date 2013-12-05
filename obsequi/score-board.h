#ifndef SCORE_BOARD_H
#define SCORE_BOARD_H

class Board;

int does_next_player_win(Board* board, bool print);
int does_who_just_moved_win(Board* board, bool print);

#endif  // SCORE_BOARD_H
