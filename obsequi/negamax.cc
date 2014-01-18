#include "negamax.h"

#include <stdio.h>
#include "board.h"
#include "hash-table.h"
#include "move.h"
#include "score-board.h"
#include "stats.h"

namespace obsequi {

//#################################################################
// Negamax and driver functions. (and function prototype).
//#################################################################
static int negamax(int depth, int remaining, Board* curr, ObsequiStats* stats,
                   int alpha, int beta);

//=================================================================
// Search for move function. (Negamax Driver)
//=================================================================
extern int
search_for_move(Board* curr, ObsequiStats* stats, int *row, int *col) {
  int  i, value = 0;

  // Can we already determine a winner?
  int rv;
  if (curr->IsGameOver(&rv)) {
    *col = *row = -1;
    return rv;
  }

  MoveList movelist;
  movelist.GenerateAllMoves(curr);

  // Really this is for iterative deepening.
  for (int d = 1; d < 50; d += 44) {

    // Initialize alpha and beta.
    int alpha = -5000, beta = 5000;

    // Re-initialize the statistics for each iteration.
    *stats = ObsequiStats();

    // iterate through all the possible moves.
    for (i = 0; i < movelist.Size(); i++) {
      Move& move = movelist[i];
      //set_position_values();

      //curr->position->Print();
      //curr->GetOpponent()->position->Print();

      stats->level_[0].node_count_++;
      stats->level_[0].curr_move_ = i;

      curr->ApplyMove(move);
      check_hash_code_sanity(curr->GetHashKeys());

      value = -negamax(1, d-1, curr->GetOpponent(), stats, -beta, -alpha);

      curr->UndoMove(move);
      check_hash_code_sanity(curr->GetHashKeys());

      char buffer[80];
      printf("Move (%d,%d), value %d: %s.\n",
             move.array_index, move.mask_index,
             value, u64bit_to_string(stats->node_count_, buffer));
      printf("alpha %d, beta %d.\n", alpha, beta);

      move.info = value;

      if (value >= beta) {
        alpha = value;
        break;
      }
      if (value > alpha) {
        alpha = value;
      }
    }

    if (value >= 5000) {
      printf("Winner found: %d.\n", value);
      *row = movelist[i].array_index;
      *col = movelist[i].mask_index;

      stats->PrintStats();

      return value;
    }

    // remove lossing moves from movelist.
    /*  iterative deepening...
    {
      int rem = 0;
      for (i = 0; i < num_moves; i++) {
        if (movelist[i].info <= -5000) rem++;
        else if (rem > 0) movelist[i-rem] = movelist[i];
      }
      num_moves -= rem;
    }
    */

    stats->PrintStats();

    //if (num_moves == 0) {
    //  break;
    //}

    // Iterative deepening
    //sort_moves(movelist, 0, num_moves);

    printf("The value is %d at a depth of %d.\n", value, d);
    printf("Nodes: %lu.\n", stats->node_count_);
  }

  *col = *row = -1;

  return value;
}


//=================================================================
// Negamax Function.
//=================================================================
static int negamax(int depth, int remaining, Board* curr, ObsequiStats* stats,
                   int alpha, int beta) {
  int value;
  int init_alpha = alpha, init_beta = beta;
  Move   forcefirst;
  ObsequiLevelStats* level_stats = &stats->level_[depth];

  // increment a couple of stats
  stats->node_count_++;
  level_stats->node_count_++;

  u64bit start_nodes = stats->node_count_;

  // if no depth remaining stop search.
  if (remaining <= 0) {
    int rv;
    is_game_over_expensive(*curr, *curr->GetOpponent(), &rv);
    return rv;
  }

  //------------------------------------------
  // Can we determine a winner yet (simple check).
  //------------------------------------------
  int rv;
  if (curr->IsGameOver(&rv)) {
    /*
    static int pc = 0;
    if (pc < 10) {
      printf("You lose.\n");
      curr->Print();
      pc++;
    }*/
    level_stats->cut_simple_++;
    return rv;
  }

  //------------------------------------------
  // check transposition table
  //------------------------------------------
  forcefirst.array_index = -1;
  if (hashlookup(curr->GetHashKeys(), &value, &alpha, &beta, remaining,
                 &forcefirst)) {
    level_stats->cut_transp_++;
    return value;
  }
  // since we aren't using iter deep not interested in forcefirst.
  forcefirst.array_index = -1;


  //------------------------------------------
  // Can we determine a winner yet (look harder).
  //------------------------------------------
  if (is_game_over_expensive(*curr, *curr->GetOpponent(), &rv)) {
    level_stats->cut_expensive_++;
    return rv;
  }

  const Move* m;
  MoveList movelist;
  Move best = movelist[0];

  while ((m = movelist.GetNext(curr)) != nullptr) {
    u64bit poor_move_cost = stats->node_count_ - start_nodes;

    // A few statistics
    level_stats->curr_move_ = movelist.Index();

    curr->ApplyMove(*m);
    value = -negamax(depth+1, remaining-1, curr->GetOpponent(), stats,
                     -beta, -alpha);
    curr->UndoMove(*m);

    // If this is a cutoff, break.
    if (value >= beta) {
      alpha = value;
      best  = *m;

      level_stats->win_count_++;
      if (movelist.Index() < 5) {
        level_stats->win_move_[movelist.Index()]++;
      } else {
        level_stats->win_move_[5]++;
      }
      level_stats->poor_move_cost_ += poor_move_cost;

      /*
      if (i > 0 && depth < 7) {
        printf("Really Bad First move.\n");
        curr->Print();
        for (int j = 0; j < movelist.length(); j++) {
          printf("Move %d: Row (%d,%d), Score: %d %lu %s\n",
              j, movelist[j].array_index, movelist[j].mask_index,
              movelist[j].info, stats->node_count_ - start_nodes,
              (i == j) ? "(*)" : "");
        }
      }
      */
      break;
    }

    // If the current value is greater than alpha, increase alpha.
    if (value > alpha) {
      alpha = value;
      best = *m;
    }
  }

  // save the position in the hashtable
  hashstore(curr->GetHashKeys(), alpha, init_alpha, init_beta,
            (stats->node_count_ - start_nodes) >> 5,
            remaining, best);

  return alpha;
}

}  // namespace obsequi
