
#include "globals.h"
#include "move.h"
#include "positional-values.h"
#include "hash-table.h"
#include "countbits.h"

#include <time.h>
#include <ctype.h>
#include <algorithm>

//#define RECORD_MOVES
//#define DEBUG_NEGAMAX

//=================================================================
// Variables used for statistics gathering.
//=================================================================
static s32bit cut1 = 0, cut2 = 0, cut3 = 0, cut4 = 0;
static s32bit stat_cutoffs[40];
static s32bit stat_nodes[40];
static s32bit stat_nth_try[40][10];

//#################################################################
// Other variables.
//#################################################################
s32bit         g_empty_squares = 0;

// Stats
u64bit         g_num_nodes;
s32bit         g_move_number[128];
static s32bit g_starting_depth;

#ifdef RECORD_MOVES
static s32bit  g_move_player[128];
static Move    g_move_position[128];
#endif

Board* g_boardx[2];
s32bit g_board_size[2] = {-1,-1};


//=================================================================
// Print the statistics which we have gathered.
//=================================================================
static void
print_stats()
{
  s32bit i, j;

  printf("%d %d %d %d.\n\n", cut1, cut2, cut3, cut4);

  for(i = 0; i < 40; i++){
    if(stat_cutoffs[i] != 0 || stat_nodes[i] != 0){
      printf("cutoffs depth %d: (%d) %d -",
             i, stat_nodes[i], stat_cutoffs[i]);
      for(j = 0; j < 5; j++)
        printf(" %d", stat_nth_try[i][j]);
      printf(" >%d.\n", stat_nth_try[i][5]);
    }
  }
}

//=================================================================
// Initialize the statistical variables.
//=================================================================
static void
init_stats()
{
  s32bit i, j;

  // zero all data.
  for(i = 0; i < 40; i++){
    for(j = 0; j < 6; j++)
      stat_nth_try[i][j] = 0;
    stat_cutoffs[i] = 0;
    stat_nodes[i] = 0;
  }
}

extern void
initialize_board(s32bit num_rows, s32bit num_cols, s32bit board[30][30])
{
  // Check if we need to re-initialize the solver.
  //bool init = 1; //(g_trans_table == NULL || !horz || !vert ||
              // horz->num_rows != num_rows || vert->num_rows != num_cols);

  g_board_size[HORIZONTAL] = num_rows;
  g_board_size[VERTICAL]   = num_cols;

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

  init_hashtable(num_rows, num_cols, board);
}

extern const char*
current_search_state()
{
  static char* str = NULL;

  if(str != NULL) free(str);

  int x = asprintf(&str, "Nodes: %s.\n%d %d %d %d %d %d %d %d %d %d %d %d.",
                   u64bit_to_string(g_num_nodes),
                   g_move_number[0], g_move_number[1], g_move_number[2],
                   g_move_number[3], g_move_number[4], g_move_number[5],
                   g_move_number[6], g_move_number[7], g_move_number[8],
                   g_move_number[9], g_move_number[10], g_move_number[11]);
  if (x == -1) exit(3);

  return str;
}


//#################################################################
// Negamax and driver functions. (and function prototype).
//#################################################################
static s32bit
negamax(s32bit depth_remaining, s32bit whos_turn_t, s32bit alpha, s32bit beta);

bool wayToSort(Move i, Move j) { return i.info > j.info; }

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

  g_empty_squares = 0;
  for(i = 0; i < curr->GetNumRows(); i++)
    g_empty_squares += countbits32( ~(curr->board[i+1]) );

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
    g_num_nodes = 0;
    init_stats();

    // set what the starting max depth is.
    g_starting_depth = d;

    // iterate through all the possible moves.
    for(i = 0; i < movelist.length(); i++){
      Move& move = movelist[i];
      //set_position_values();

      //curr->position->Print();
      //curr->GetOpponent()->position->Print();

      g_move_number[0] = i;
#ifdef RECORD_MOVES
      g_move_player[0] = whos_turn;
      g_move_position[0] = move;
#endif

      g_empty_squares -= 2;
      curr->ToggleMove(move);
      toggle_hash_code(whos_turn, move);
      check_hash_code_sanity();

      value = -negamax(d-1, whos_turn^PLAYER_MASK, -beta, -alpha);

      g_empty_squares += 2;
      curr->ToggleMove(move);
      toggle_hash_code(whos_turn, move);
      check_hash_code_sanity();

      printf("Move (%d,%d), value %d: %s.\n",
             move.array_index, move.mask_index,
             value, u64bit_to_string(g_num_nodes));
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

      *nodes = g_num_nodes;

      print_stats();

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

    print_stats();

    //if(num_moves == 0){
    //  break;
    //}

    // Iterative deepening
    //std::stable_sort(movelist, movelist+num_moves, wayToSort);
    //std::__inplace_stable_sort(movelist, movelist+num_moves, wayToSort);
    //sort_moves(movelist, 0, num_moves);

    printf("The value is %d at a depth of %d.\n", value, d);
    printf("Nodes: %lu.\n", g_num_nodes);
  }

  *col = *row = -1;
  *nodes = g_num_nodes;

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
  u64bit start_nodes = g_num_nodes;
  Move   forcefirst;

  Board* curr = g_boardx[whos_turn];

  // increment a couple of stats
  g_num_nodes++;
  stat_nodes[g_starting_depth - depth_remaining]++;

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
    cut1++;
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
    cut3++;
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
      g_move_number[g_starting_depth - depth_remaining] = i;
#ifdef RECORD_MOVES
      g_move_player[g_starting_depth - depth_remaining] = whos_turn;
      g_move_position[g_starting_depth - depth_remaining] = movelist[i];
#endif

      // make move.
      g_empty_squares -= 2;
      curr->ToggleMove(movelist[i]);
      toggle_hash_code(whos_turn, movelist[i]);

      // recurse.
      value = -negamax(depth_remaining-1,whos_turn^PLAYER_MASK, -beta, -alpha);

      // undo move.
      g_empty_squares += 2;
      curr->ToggleMove(movelist[i]);
      toggle_hash_code(whos_turn, movelist[i]);

      // If this is a cutoff, break.
      if(value >= beta){
        alpha = value;
        best  = movelist[i];

        stat_cutoffs[g_starting_depth - depth_remaining]++;
        if(i < 5) stat_nth_try[g_starting_depth - depth_remaining][i]++;
        else stat_nth_try[g_starting_depth - depth_remaining][5]++;
        
        /*
        int depth = g_starting_depth - depth_remaining;
        if (i > 0 && depth < 12) {
          printf("Really Bad First move.\n");
          curr->Print();
          for (int j = 0; j < movelist.length(); j++) {
            printf("Move %d: Row (%d,%d), Score: %d %lu %s\n",
                j, movelist[j].array_index, movelist[j].mask_index,
                movelist[j].info, g_num_nodes - start_nodes, (i == j) ? "(*)" : "");
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
  hashstore(alpha, init_alpha, init_beta, (g_num_nodes - start_nodes) >> 5,
            depth_remaining, best, whos_turn);

  return alpha;
}
