#include "stats.h"

#include <string.h>
#include <stdio.h>
#include "base.h"

namespace obsequi {

ObsequiStats::ObsequiStats() {
  // We should zero everything...
  node_count_ = 0;
  memset(level_, 0, sizeof(level_));
}

void ObsequiStats::PrintStats() {
  //printf("%d %ld.\n\n", game_over_simple_, game_over_expensive_);

  for(int i = 0; i < MAX_SEARCH_DEPTH; i++){
    ObsequiLevelStats* stats = &level_[i];

    if (stats->node_count_ == 0) continue;

    printf("depth %2d: Nodes (%10ld) Bad (%10ld) ",
           i, stats->node_count_, stats->poor_move_cost_);
    printf("Cuts (S %ld, E %ld T %ld) ",
           stats->cut_simple_, stats->cut_expensive_, stats->cut_transp_);
    printf("Win Move ");
    for(int j = 0; j < 5; j++)
      printf("%ld ", stats->win_move_[j]);
    printf(">%ld.\n", stats->win_move_[5]);
  }
}

void ObsequiStats::PrintSearchState() {
  char buffer[80];
  printf("Nodes: %s.\n", u64bit_to_string(node_count_, buffer));
  printf("%d %d %d %d %d %d %d %d %d %d %d %d.\n",
         level_[0].curr_move_,
         level_[1].curr_move_,
         level_[2].curr_move_,
         level_[3].curr_move_,
         level_[4].curr_move_,
         level_[5].curr_move_,
         level_[6].curr_move_,
         level_[7].curr_move_,
         level_[8].curr_move_,
         level_[9].curr_move_,
         level_[10].curr_move_,
         level_[11].curr_move_);
}

}  // namespace obsequi
