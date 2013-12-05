//########################################################
// Constants which we use.
// Any constants which we commonly change or try to tweak
//   should be in cppflags.raw.
//########################################################

// max number of empty squares which should ever exist is 256
//  therefore we could possible get close to this number of moves.
#define MAXMOVES 256
#define MAX_MOVES 256

// ...
#define MAX_ROWS 32

// Player stuff. (Used for example with g_board to get the current player).
#define HORIZONTAL    0
#define VERTICAL      1
#define PLAYER_MASK   1

// Used with g_board_size (to get num of rows and num of cols).
#define ROW_INDEX  HORIZONTAL
#define COL_INDEX  VERTICAL
