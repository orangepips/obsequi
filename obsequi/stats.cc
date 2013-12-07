#include "stats.h"

#include <stdio.h>
#include "utils.h"

ObsequiStats::ObsequiStats() {
  // We should zero everything...
}

void ObsequiStats::PrintStats() {
  printf("%d %d.\n\n", game_over_simple_, game_over_expensive_);

  for(int i = 0; i < 40; i++){
    if(depth_cutoffs_[i] != 0 || depth_nodes_[i] != 0){
      printf("cutoffs depth %d: (%d) %d -",
             i, depth_nodes_[i], depth_cutoffs_[i]);
      for(int j = 0; j < 5; j++)
        printf(" %d", depth_nth_try_[i][j]);
      printf(" >%d.\n", depth_nth_try_[i][5]);
    }
  }
}

void ObsequiStats::PrintSearchState() {
  printf("Nodes: %s.\n%d %d %d %d %d %d %d %d %d %d %d %d.\n",
         u64bit_to_string(num_nodes_),
         move_number_[0], move_number_[1], move_number_[2],
         move_number_[3], move_number_[4], move_number_[5],
         move_number_[6], move_number_[7], move_number_[8],
         move_number_[9], move_number_[10], move_number_[11]);
}
