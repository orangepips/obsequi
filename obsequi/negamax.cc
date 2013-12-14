
#include "globals.h"
#include "move.h"
#include "positional-values.h"
#include "hash-table.h"
#include "countbits.h"
#include "stats.h"

#include <time.h>
#include <ctype.h>
#include <algorithm>

//#define DEBUG_NEGAMAX

Board* g_boardx[2];
s32bit g_board_size[2] = {-1,-1};
s32bit g_starting_depth;
ObsequiStats g_stats;

extern void
current_search_state()
{
  g_stats.PrintSearchState();
}

//=================================================================
// Print the statistics which we have gathered.
//=================================================================
extern void
initialize_board(s32bit num_rows, s32bit num_cols, s32bit board[30][30])
{
  // Check if we need to re-initialize the solver.
  //bool init = 1; //(g_trans_table == NULL || !horz || !vert ||
              // horz->num_rows != num_rows || vert->num_rows != num_cols);

  g_board_size[HORIZONTAL] = num_rows;
  g_board_size[VERTICAL]   = num_cols;

  init_hashtable(num_rows, num_cols, board);

  Board* horz = g_boardx[HORIZONTAL] = new Board(num_rows, num_cols);
  g_boardx[VERTICAL] = horz->GetOpponent();

  for(int i = 0; i < num_rows; i++) {
    for(int j = 0; j < num_cols; j++) {
      if(board[i][j] != 0){
        horz->SetBlock(i, j);
      }
    }
  }

  horz->Print();
  printf("\n");
  horz->PrintInfo();

  check_hash_code_sanity();
}


//#################################################################
// Negamax and driver functions. (and function prototype).
//#################################################################
static s32bit
negamax(s32bit depth_remaining, s32bit whos_turn_t, s32bit alpha, s32bit beta);

//=================================================================
// Search for move function. (Negamax Driver)
//=================================================================
extern s32bit
search_for_move(char dir, s32bit *row, s32bit *col, u64bit *nodes)
{
  s32bit  d, i, value = 0;
  s32bit  whos_turn;

  // Set who's turn it is.
  if(toupper(dir) == 'V')      whos_turn = VERTICAL;
  else if(toupper(dir) == 'H') whos_turn = HORIZONTAL;
  else { fatal_error(1, "Invalid player.\n"); exit(1); }

  Board* curr = g_boardx[whos_turn];

  curr->shared->empty_squares = 0;
  for(i = 0; i < curr->GetNumRows(); i++)
    curr->shared->empty_squares += countbits32( ~(curr->board[i+1]) );

  // Can we already determine a winner?
  int rv;
  if (curr->IsGameOver(&rv)) {
    *col = *row = -1;
    *nodes = 0;
    return rv;
  }

  MoveList movelist;
  movelist.GenerateAllMoves(curr);

  // Really this is for iterative deepening.
  for(d = 1; d < 50; d += 44){

    // Initialize alpha and beta.
    s32bit alpha = -5000, beta = 5000;

    // Re-initialize the statistics for each iteration.
    g_stats.num_nodes_ = 0;
    g_stats = ObsequiStats();

    // set what the starting max depth is.
    g_starting_depth = d;

    // iterate through all the possible moves.
    for(i = 0; i < movelist.length(); i++){
      Move& move = movelist[i];
      //set_position_values();

      //curr->position->Print();
      //curr->GetOpponent()->position->Print();

      g_stats.move_number_[0] = i;

      curr->shared->empty_squares -= 2;
      curr->ToggleMove(move);
      g_hashkey.Xor(curr->GetHashKeys(move));
      check_hash_code_sanity();
      //exit(1);

      value = -negamax(d-1, whos_turn^PLAYER_MASK, -beta, -alpha);

      curr->shared->empty_squares += 2;
      curr->ToggleMove(move);
      g_hashkey.Xor(curr->GetHashKeys(move));
      check_hash_code_sanity();

      printf("Move (%d,%d), value %d: %s.\n",
             move.array_index, move.mask_index,
             value, u64bit_to_string(g_stats.num_nodes_));
      printf("alpha %d, beta %d.\n", alpha, beta);

      move.info = value;

      if(value >= beta){
        alpha = value;
        break;
      }
      if(value > alpha) {
        alpha = value;
      }
    }

    if(value >= 5000){
      printf("Winner found: %d.\n", value);
      if(whos_turn == HORIZONTAL){
        *row = movelist[i].array_index;
        *col = movelist[i].mask_index;
      } else if(whos_turn == VERTICAL){
        *col = movelist[i].array_index;
        *row = movelist[i].mask_index;
      } else {
        fatal_error(1, "oops.");
      }

      *nodes = g_stats.num_nodes_;

      g_stats.PrintStats();

      return value;
    }

    // remove lossing moves from movelist.
    /*  iterative deepening...
    {
      s32bit rem = 0;
      for(i = 0; i < num_moves; i++){
        if(movelist[i].info <= -5000) rem++;
        else if(rem > 0) movelist[i-rem] = movelist[i];
      }
      num_moves -= rem;
    }
    */

    g_stats.PrintStats();

    //if(num_moves == 0){
    //  break;
    //}

    // Iterative deepening
    //sort_moves(movelist, 0, num_moves);

    printf("The value is %d at a depth of %d.\n", value, d);
    printf("Nodes: %lu.\n", g_stats.num_nodes_);
  }

  *col = *row = -1;
  *nodes = g_stats.num_nodes_;

  return value;
}


//=================================================================
// Negamax Function.
//=================================================================
static s32bit
negamax(s32bit depth_remaining, s32bit whos_turn_t, s32bit alpha, s32bit beta)
{
  s32bit whos_turn = whos_turn_t & PLAYER_MASK;
  s32bit value;
  s32bit init_alpha = alpha, init_beta = beta;
  u64bit start_nodes = g_stats.num_nodes_;
  Move   forcefirst;

  Board* curr = g_boardx[whos_turn];

  // increment a couple of stats
  g_stats.num_nodes_++;
  g_stats.depth_nodes_[g_starting_depth - depth_remaining]++;

  // if no depth remaining stop search.
  if( depth_remaining <= 0 ){
    int rv;
    curr->IsGameOverExpensive(&rv);
    return rv;
  }

  //------------------------------------------
  // Can we determine a winner yet (simple check).
  //------------------------------------------
  int rv;
  if (curr->IsGameOver(&rv)) {
    g_stats.game_over_simple_++;
    return rv;
  }

  //------------------------------------------
  // check transposition table
  //------------------------------------------

  forcefirst.array_index = -1;
  if(hashlookup(&value, &alpha, &beta, depth_remaining,
                &forcefirst, whos_turn))
    return value;
  // since we aren't using iter deep not interested in forcefirst.
  forcefirst.array_index = -1;


  //------------------------------------------
  // Can we determine a winner yet (look harder).
  //------------------------------------------
  if (curr->IsGameOverExpensive(&rv)) {
    g_stats.game_over_expensive_++;
    return rv;

/*
#ifdef DEBUG_NEGAMAX
    if(random() % 1000000 == -1){
      does_next_player_win(curr, 1);
      print_board(whos_turn);
    }
#endif
*/
  }

  int i = 0;
  MoveList movelist;
  movelist.GenerateNextMoves(curr, forcefirst);
  Move best = movelist[0];

  do {
    // Iterate through all the moves.
    for(; i < movelist.length(); i++){

      // A few statistics
      g_stats.move_number_[g_starting_depth - depth_remaining] = i;

      // make move.
      curr->shared->empty_squares -= 2;
      curr->ToggleMove(movelist[i]);
      g_hashkey.Xor(curr->GetHashKeys(movelist[i]));

      // recurse.
      value = -negamax(depth_remaining-1,whos_turn^PLAYER_MASK, -beta, -alpha);

      // undo move.
      curr->shared->empty_squares += 2;
      curr->ToggleMove(movelist[i]);
      g_hashkey.Xor(curr->GetHashKeys(movelist[i]));

      // If this is a cutoff, break.
      if(value >= beta){
        alpha = value;
        best  = movelist[i];

        g_stats.depth_cutoffs_[g_starting_depth - depth_remaining]++;
        if(i < 5) g_stats.depth_nth_try_[g_starting_depth - depth_remaining][i]++;
        else g_stats.depth_nth_try_[g_starting_depth - depth_remaining][5]++;
        
        /*
        int depth = g_starting_depth - depth_remaining;
        if (i > 0 && depth < 12) {
          printf("Really Bad First move.\n");
          curr->Print();
          for (int j = 0; j < movelist.length(); j++) {
            printf("Move %d: Row (%d,%d), Score: %d %lu %s\n",
                j, movelist[j].array_index, movelist[j].mask_index,
                movelist[j].info, g_stats.num_nodes_ - start_nodes, (i == j) ? "(*)" : "");
          }
        }
        */
        break;
      }

      // If the current value is greater than alpha, increase alpha.
      if(value > alpha) {
        alpha = value;
        best  = movelist[i];
      }
    }
    // If we have broken out of previous FOR loop make sure we break out
    //   of this loop as well.
    if(value >= beta) break;

  } while(movelist.GenerateNextMoves(curr, forcefirst));

  // save the position in the hashtable
  hashstore(alpha, init_alpha, init_beta, (g_stats.num_nodes_ - start_nodes) >> 5,
            depth_remaining, best, whos_turn);

  return alpha;
}
