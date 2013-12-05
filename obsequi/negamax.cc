
#include "globals.h"
#include "macros.h"
#include "move-gen.h"

#include <time.h>
#include <ctype.h>


//#################################################################
// Statistics gathering variables and functions.
//#################################################################

//=================================================================
// Variables used for statistics gathering.
//=================================================================
static s32bit cut1 = 0, cut2 = 0, cut3 = 0, cut4 = 0;
static s32bit stat_cutoffs[40];
static s32bit stat_nodes[40];
static s32bit stat_nth_try[40][10];

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


//#################################################################
// Other variables.
//#################################################################

//#define RECORD_MOVES
//#define DEBUG_NEGAMAX


extern s32bit  debug_score_move;

Hash_Key       g_norm_hashkey;
Hash_Key       g_flipV_hashkey;
Hash_Key       g_flipH_hashkey;
Hash_Key       g_flipVH_hashkey;

u64bit         g_num_nodes;

s32bit         g_first_move[2][32][32];
s32bit         g_empty_squares = 0;

s32bit         g_move_number[128];

#ifdef RECORD_MOVES
static s32bit  g_move_player[128];
static Move    g_move_position[128];
#endif

s32bit         g_print = 0;


static s32bit starting_depth;


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
  s32bit  d, i, value = 0, num_moves;
  Move    movelist[MAXMOVES];
  s32bit  whos_turn;
  Move    forcefirst;
    
  // Set who's turn it is.
  if(toupper(dir) == 'V')      whos_turn = VERTICAL;
  else if(toupper(dir) == 'H') whos_turn = HORIZONTAL;
  else { fatal_error(1, "Invalid player.\n"); exit(1); }

  Board* curr = g_boardx[whos_turn];

  // initialize the number of empty squares.
  g_empty_squares = 0;
  for(i = 0; i < curr->GetNumRows(); i++)
    g_empty_squares += countbits32( ~(curr->board[i+1]) );
  
  // zero out all the statistics variables.
  init_stats();
  
  // Can we already determine a winner?
  int rv;
  if (curr->IsGameOver(&rv)) {
    *col = *row = -1;
    *nodes = 0;
    return rv;
  }
  
  // generate all possible moves for current player given current position.
  num_moves = move_generator(*curr, movelist);

  // This should never happen.
  CHECK(num_moves, "No moves.");

  // should possibly sort the whole list instead of just get first.
  forcefirst.array_index = -1;
  score_and_get_first(curr, movelist, num_moves, whos_turn, forcefirst);
  sort_moves(movelist, 1, num_moves);

  // Really this is for iterative deepening.
  for(d = 1; d < 50; d += 44){

    // Initialize alpha and beta.
    s32bit alpha = -5000, beta = 5000;

    // Re-initialize the statistics for each iteration.
    g_num_nodes = 0;
    init_stats();

    // set what the starting max depth is.
    starting_depth = d;
        
    // iterate through all the possible moves.
    for(i = 0; i < num_moves; i++){

#ifdef DYNAMIC_POSITION_VALUES
      init_move_value();
      set_move_value(movelist[i], whos_turn);
#else
      set_position_values();
#endif
      
      g_move_number[0] = i;
#ifdef RECORD_MOVES
      g_move_player[0] = whos_turn;
      g_move_position[0] = movelist[i];
#endif
      
      g_empty_squares -= 2;
      curr->ToggleMove(movelist[i]);
      toggle_hash_code
    (g_keyinfo[whos_turn][movelist[i].array_index][movelist[i].mask_index]);
      check_hash_code_sanity();
      
      value = -negamax(d-1, whos_turn^PLAYER_MASK, -beta, -alpha);
      
#ifdef DYNAMIC_POSITION_VALUES
      unset_move_value(movelist[i], whos_turn);
#endif

      g_empty_squares += 2;
      curr->ToggleMove(movelist[i]);
      toggle_hash_code
    (g_keyinfo[whos_turn][movelist[i].array_index][movelist[i].mask_index]);
      check_hash_code_sanity();

      printf("Move (%d,%d), value %d: %s.\n",
             movelist[i].array_index, movelist[i].mask_index,
             value, u64bit_to_string(g_num_nodes));
      printf("alpha %d, beta %d.\n", alpha, beta);

      movelist[i].info = value;

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
    {
      s32bit rem = 0;
      for(i = 0; i < num_moves; i++){
        if(movelist[i].info <= -5000) rem++;
        else if(rem > 0) movelist[i-rem] = movelist[i];
      }
      num_moves -= rem;
      /*
      for(i = 0; i < num_moves; i++){
        printf("(%d,%d): %d.\n",
               movelist[i].array_index, movelist[i].mask_index,
               movelist[i].info);
      }
      */
    }

    print_stats();
    
    if(num_moves == 0){
      break;
    }
    
    // use a stable sort algorithm
    {
      Move swp;
      s32bit max, index, j;
      
      for(i=0; i<num_moves; i++) {
        max = movelist[i].info;
        index = i;
        
        for(j=i+1; j < num_moves; j++)
          if(movelist[j].info > max){
            max = movelist[j].info;
            index = j;
          }
    
        if(index != i){
          swp = movelist[index];
          //          printf("%d %d\n", index, i);
          for(j = index; j != i; j--){
            movelist[j] = movelist[j-1];
          }
          movelist[i] = swp;
        }
      }
    }
    
    printf("The value is %d at a depth of %d.\n", value, d);
    printf("Nodes: %u.\n", (u32bit)g_num_nodes);
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
  Move   movelist[MAXMOVES], best;
  s32bit whos_turn = whos_turn_t & PLAYER_MASK;
  s32bit value;
  s32bit init_alpha = alpha, init_beta = beta;
  u32bit start_nodes = g_num_nodes;
  Move   forcefirst;

  s32bit stage = 0, state = 0, true_count, i = 0, num_moves = 1;
  
  Board* curr = g_boardx[whos_turn];

#ifdef DYNAMIC_POSITION_VALUES
  s32bit dyn_set;
#endif

  // increment a couple of stats
  g_num_nodes++;
  stat_nodes[starting_depth - depth_remaining]++;

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

{  
  int rv;
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
}

  //------------------------------------------
  // Generate child nodes and examine them.
  //------------------------------------------

  // initialize a few variables. (some of them don't really need to be.)
  stage = state = true_count = i = 0;
  num_moves = 1;

#ifdef TWO_STAGE_GENERATION
  true_count = move_generator_stage1(*curr, movelist);
  if(true_count == 0){
    true_count = move_generator_stage2(*curr, 0, movelist);
    stage = 1;
    if(true_count == 0) fatal_error(1, "Should always have a move.\n");
  }
#else
  true_count = move_generator(*curr, movelist);
  stage = 1;
  if(true_count == 0)   fatal_error(1, "Should always have a move.\n");
#endif
  
  // score all the moves and move the best to the front.
  score_and_get_first(curr, movelist, true_count, whos_turn, forcefirst);
  
  best = movelist[0];
  
  // need to sort moves and generate more moves in certain situations.
  while(state < 3){
    if(state == 0) {  // state 0 - play first move.
      state = 1;
    } else if(state == 1){  // state 1 - sort the moves and play the rest.
      sort_moves(movelist, 1, true_count);
      num_moves = true_count;
      if(stage == 0) state = 2;
      else           state = 3;
    } else { // state 2 - generate the second set of moves and play them.
      num_moves = move_generator_stage2(*curr, num_moves, movelist);
      state = 3;
    }
    
    // Iterate through all the moves.
    for(; i < num_moves; i++){

      // A few statistics
      g_move_number[starting_depth - depth_remaining] = i;
#ifdef RECORD_MOVES
      g_move_player[starting_depth - depth_remaining] = whos_turn;
      g_move_position[starting_depth - depth_remaining] = movelist[i];
#endif

      // make move.
      g_empty_squares -= 2;
      curr->ToggleMove(movelist[i]);
      toggle_hash_code
    (g_keyinfo[whos_turn][movelist[i].array_index][movelist[i].mask_index]);
#ifdef DYNAMIC_POSITION_VALUES
      dyn_set = set_move_value(movelist[i], whos_turn);
#endif
      
      // recurse.
      value = -negamax(depth_remaining-1,whos_turn^PLAYER_MASK,
                       -beta, -alpha);
      
      // undo move.
      g_empty_squares += 2;
      curr->ToggleMove(movelist[i]);
      toggle_hash_code
    (g_keyinfo[whos_turn][movelist[i].array_index][movelist[i].mask_index]);
#ifdef DYNAMIC_POSITION_VALUES
      if(dyn_set != 0) unset_move_value(movelist[i], whos_turn);
#endif

      // If this is a cutoff, break.
      if(value >= beta){
        alpha = value;
        best  = movelist[i];
        
        stat_cutoffs[starting_depth - depth_remaining]++;
        if(i < 5) stat_nth_try[starting_depth - depth_remaining][i]++;
        else      stat_nth_try[starting_depth - depth_remaining][5]++;
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
  }
  
  // save the position in the hashtable
  hashstore(alpha, init_alpha, init_beta, (g_num_nodes - start_nodes) >> 5,
            depth_remaining, best, whos_turn);
  
  return alpha;
}
