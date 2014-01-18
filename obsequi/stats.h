#ifndef OBSEQUI_STATS_H
#define OBSEQUI_STATS_H

#include "base.h"

namespace obsequi {

struct ObsequiLevelStats {
  long int node_count_;
  long int poor_move_cost_;
  long int win_count_;
  long int win_move_[10];

  long int cut_simple_;
  long int cut_expensive_;
  long int cut_transp_;

  // TODO(nathanbullock): Should we record more, like the move made?
  int curr_move_;
};

class ObsequiStats {
 public:
  ObsequiStats();

  void PrintStats();
  void PrintSearchState();

  long int node_count_;
  ObsequiLevelStats level_[MAX_SEARCH_DEPTH];
};

}  // namespace obsequi
#endif  // OBSEQUI_STATS_H
