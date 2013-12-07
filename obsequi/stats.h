#ifndef STATS_H
#define STATS_H

struct ObsequiStats {
  ObsequiStats();
  ~ObsequiStats() {}

  void PrintStats();
  void PrintSearchState();

  // Total nodes searched.
  long int num_nodes_;

  // Depth based stats.
  int depth_cutoffs_[60];
  int depth_nodes_[60];
  int depth_nth_try_[60][10];

  // Cuts made because of:
  int depth_cut_;
  int game_over_simple_;
  int game_over_expensive_;

  // Current search state info.
  // TODO(nathanbullock): Should we record more, like the move made?
  int move_number_[60];
};

#endif // STATS_H
