#ifndef NEGAMAX_H
#define NEGAMAX_H

namespace obsequi {

class Board;
class ObsequiStats;

// This can take a long, long time to run.
// curr, is a board position we would like to solve for, curr moves next.
// stats, is used to return certain statics about the solver.
// row and col return the move that was able to win, otherwise undefined.
// Function returns +5000 if curr wins, -5000 if curr loses, something else if 
// we weren't able to determine a winner.
int search_for_move(Board* curr, ObsequiStats* stats, int *row, int *col);

}  // namespace obsequi
#endif // NEGAMAX_H
