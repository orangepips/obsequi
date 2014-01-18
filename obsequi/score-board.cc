#include "score-board.h"

#include <iostream>
#include "obsequi.pb.h"
#include "board.h"
#include "base.h"

using namespace std;

namespace obsequi {

//#################################################################
// This function tries to pack as many 'protective' areas onto
//  the board as it can. (no guarantee of optimality).
//#################################################################
static inline void pack_prot(
    int num_rows, const u32bit board[], u32bit tmp_board[], int* prot) {
  int p = 0;  // number of prot. areas

  for (int r = 0; r < num_rows - 1; r++) {
    // set bit at each position where there is interference.
    u32bit inter = (tmp_board[r] | tmp_board[r+1] |
                    tmp_board[r+2] | tmp_board[r+3] |
                    board[r+1] | board[r+2]);
    inter = (inter >> 1) | inter;

    // set bit at each position where there is a wall.
    u32bit walls = ( ((board[r] >> 1) & board[r]) |
                     ((board[r+3] >> 1) & board[r+3]) );

    // combine (bit set at each position which we can place a prot. area).
    inter = walls & ~inter;

    // process the positions.
    while (inter) {
      u32bit tmp = (inter & -inter);  // least sig bit of m
      inter &= ~(tmp | (tmp << 1));  // remove bit and next bit.

      tmp_board[r+1] |= tmp | (tmp << 1); //record where we put
      tmp_board[r+2] |= tmp | (tmp << 1); // the protective square.

      p++; // count it.
    }
  }

  *prot = p;
}

//#################################################################
// This function tries to pack as many 'vuln. type 1 and 2' areas
//  onto the board as it can. (no guarantee of optimality).
// It also counts the number of unused squares.
//#################################################################
static inline void pack_vuln(
    int num_rows, const u32bit board[], u32bit tmp_board[],
    int* vuln2, int* vuln2_w_prot, int* vuln1, int* vuln1_w_prot, int* unused) {
  u32bit tmp = 0;
  int v2 = 0, v2_p = 0, v1 = 0, v1_p = 0, u = 0;

  u32bit s_v = 0; //start of vulnerable

  for(int r = 0; r < num_rows; r++) {
    u32bit next_prev_row  = tmp_board[r];
    next_prev_row |= ~(board[r+2] | (board[r+2] << 1));
    next_prev_row |= ~(board[r+2] | (board[r+2] >> 1));

    u32bit curr_row = ~(board[r+1] | tmp_board[r+1]);
    u32bit adj_rows = board[r] & board[r+2];
    int state = 0;

    //========================================================
    // Three types of squares, prot, empty, occ.
    //
    // state = 0
    // if (prot)  -> state = 1
    // if (empty) -> state = 2, c_t = c,
    // if (occ)   -> state = 0
    //
    // state = 1
    // if (prot)  -> state = 0, safe++
    // if (empty) -> state = 0, v_p++, mark(c-1)
    // if (occ)   -> state = 0, unu++
    //
    // state = 2
    // if (prot)  -> state = 3,
    // if (empty) -> state = 0, vuln++, mark(c_t)
    // if (occ)   -> state = 0,
    //
    // state = 3
    // if (prot)  -> state = 4, safe++
    // if (empty) -> state = 2, v_p++, mark(c_t), c_t = c
    // if (occ)   -> state = 0, v_p++, mark(c_t)
    //
    // state = 4
    // if (prot)  -> state = 3
    // if (empty) -> state = 2, c_t = c
    // if (occ)   -> state = 0
    //========================================================

    while(curr_row) {
      u32bit tmp_ = (curr_row & -curr_row);  //get least sig bit of curr_row
      curr_row ^= tmp_;               //remove least sig bit of curr_row.

      // if true then this iteration and last iteration are not contiguous.
      // Which means there was an occupied square.
      if ( ((tmp_ >> 1) & tmp) == 0 ) {
        if (state == 1) {
          u++;
          tmp_board[r+1] |= tmp;
        } else if (state == 3) {
          tmp_board[r+1] |= (s_v | (s_v << 1));
          if ((s_v & next_prev_row) || ((s_v << 1) & next_prev_row))
            v2_p++;
          else
            v1_p++;
        }
        state = 0;
      }
      tmp = tmp_;

      // empty and protected
      if ( tmp & adj_rows ) {
        if (state == 0) {
          state = 1;
        } else if (state == 1) {
          state = 0;
          //safe
        } else if (state == 2) {
          state = 3;
        } else if (state == 3) {
          state = 4;
          //safe
        } else if (state == 4) {
          state = 3;
        }
      }

      // unprotected
      else {
        if (state == 0) {
          state = 2;
          s_v = tmp;
        } else if (state == 1) {
          state = 0;

          // Check tmp >> 1

          tmp_board[r+1] |= (tmp | (tmp >> 1));
          if ((tmp & next_prev_row) || ((tmp >> 1) & next_prev_row))
            v2_p++;
          else
            v1_p++;
        } else if (state == 2) {
          state = 0;
          tmp_board[r+1] |= (s_v | (s_v << 1));
          if ((s_v & next_prev_row) || ((s_v << 1) & next_prev_row))
            v2++;
          else
            v1++;
        } else if (state == 3) {
          state = 2;
          tmp_board[r+1] |= (s_v | (s_v << 1));
          if ((s_v & next_prev_row) || ((s_v << 1) & next_prev_row))
            v2_p++;
          else
            v1_p++;
          s_v = tmp;
        } else if (state == 4) {
          state = 2;
          s_v = tmp;
        }
      }
    }

    if (state == 1) {
      u++;
      tmp_board[r+1] |= tmp;
    } else if (state == 3) {
      tmp_board[r+1] |= (s_v | (s_v << 1));
      if ((s_v & next_prev_row) || ((s_v << 1) & next_prev_row))
        v2_p++;
      else
        v1_p++;
    }
  }

  *vuln2 = v2 + v2_p;
  *vuln2_w_prot = v2_p;
  *vuln1 = v1 + v1_p;
  *vuln1_w_prot = v1_p;
  *unused = u;
}

//#################################################################
// This function tries to pack as many 'option of vulnerability' areas
//  onto the board as it can. (no guarantee of optimality).
//#################################################################
static inline void pack_safe(
    int num_rows, const u32bit board[], u32bit tmp_board[],
    int* safe_op2, int* safe_op1, int* safe_op0) {
  s32bit s2 = 0, s1 = 0, s0 = 0;

  for(int r = 0; r < num_rows; r++) {
    u32bit guard = board[r] & board[r+2];
    u32bit curr  = board[r+1] | tmp_board[r+1];

    // mask contains a bit for each safe move.
    u32bit mask  = ( (~(curr | (curr >> 1))) & (guard & (guard >> 1)) );

    while(mask) {
      u32bit tmp   =  (mask & -mask);          // least sig bit of m
      mask &= ~(tmp | (tmp << 1));      // remove bit and next bit.

      // add these bits to current.
      curr |= (tmp | (tmp << 1));

      if ( ! ( (curr | tmp_board[r] | tmp_board[r+2]) & (tmp >> 1) ) ) {
        // we have an option to move vulnerably. (which option is it).
        curr           |= (tmp >> 1);
        tmp_board[r+1] |= (tmp >> 1);

        // check up.
        if (  (!(board[r] & (tmp >> 1)))
             && (board[r-1] & (tmp >> 1)) ) {
          // up is good now check down.
          if (  (!(board[r+2] & (tmp >> 1)))
               && (board[r+3] & (tmp >> 1)) ) {
            s2++;
          } else {
            s1++;
          }
        } else if (  (!(board[r+2] & (tmp >> 1)))
                    && (board[r+3] & (tmp >> 1)) ) {
          s1++;
        } else {
          s0++;
        }
      } else
      if ( ! ( (mask | curr | tmp_board[r] | tmp_board[r+2]) & (tmp << 2) ) ) {
        // we have an option to move vulnerably. (which option is it).
        curr           |= (tmp << 2);
        tmp_board[r+1] |= (tmp << 2);

        // check up.
        if (  (!(board[r] & (tmp << 2)))
             && (board[r-1] & (tmp << 2)) ) {
          // up is good now check down.
          if (  (!(board[r+2] & (tmp << 2)))
               && (board[r+3] & (tmp << 2)) ) {
            s2++;
          } else {
            s1++;
          }
        } else if (  (!(board[r+2] & (tmp << 2)))
                    && (board[r+3] & (tmp << 2)) ) {
          s1++;
        } else {
          s0++;
        }
      }
    }
  }

  *safe_op2 = s2;
  *safe_op1 = s1;
  *safe_op0 = s0;
}

struct ScoreMetrics {
  int safe;  // Safe moves
  int opp_real;  // Opponent real moves
  int empty;  // Empty squares

  int prot;  // Protective regions

  int vuln2;  // Vulnerable moves (includes vuln_w_prot)
  int vuln2_w_prot;  // Vuln moves with square unavailable to opponent

  int vuln1;  // Vulnerable moves (includes vuln_w_prot)
  int vuln1_w_prot;  // Vuln moves with square unavailable to opponent

  int unused;  // Squares unused in packing and unavailable to opponent

  int safe_op2;  // Safe moves
  int safe_op1;  // Safe moves
  int safe_op0;  // Safe moves
};

static inline void calculate_score_metrics(
    const Board& boardx, ScoreMetrics* metrics) {
  int num_rows = boardx.GetNumRows();
  const u32bit* board = boardx.GetBoard();

  // temporary board to store which positions have already been packed.
  u32bit tmp_board[num_rows + 2];
  memset(tmp_board, 0, sizeof(tmp_board[0]) * (num_rows + 2));

  metrics->safe = boardx.GetInfo().safe;
  metrics->opp_real = boardx.GetOpponent()->GetInfo().real;
  metrics->empty = boardx.GetEmptySquares();

  // Determine the number of protective regions that we have.
  pack_prot(num_rows, board, tmp_board, &metrics->prot);

  // Determine the number of vuln, vuln_w_prot, and unused.
  pack_vuln(num_rows, board, tmp_board, &metrics->vuln2, &metrics->vuln2_w_prot,
            &metrics->vuln1, &metrics->vuln1_w_prot, &metrics->unused);

  // Determine the number of safe moves with options we have.
  pack_safe(num_rows, board, tmp_board, &metrics->safe_op2,
            &metrics->safe_op1, &metrics->safe_op0);
}

static inline void calculate_move_bounds(
    ScoreMetrics* metrics, int* moves, int* opp_moves, int* opp_moves_real) {
  if (metrics->prot % 2 == 1) {
    metrics->prot--;
    metrics->vuln2 += 2;
  }

  *moves = metrics->prot + metrics->vuln2/3 + metrics->vuln1/2 + metrics->safe;

  int x = 0;  // uh, what was x for?
  if (metrics->vuln2 % 3 != 0 && metrics->vuln1 % 2 != 0) {
    (*moves)++, metrics->unused--, x=1;
    // if (metrics->vuln2 > 0 || metrics->vuln1 > 0) metrics->unused--;
  } else if (metrics->vuln2 % 3 == 0 && metrics->vuln1 % 2 == 0) {
    x=1;
  }

  if (x == 1) {
    if (metrics->safe_op2 % 2 == 1) metrics->safe_op2--, metrics->safe_op1++;
    if (metrics->safe_op1 % 2 == 1) metrics->safe_op1--, metrics->safe_op0++;
  } else {
    if (metrics->safe_op2 % 2 == 1) {
      metrics->unused += 3;
      if (metrics->safe_op1 % 2==1) metrics->safe_op1--, metrics->safe_op0++;
    } else if (metrics->safe_op1 % 2 == 1) {
      metrics->unused += 2;
    } else if (metrics->safe_op0 % 2 == 1) {
      metrics->unused += 1;
    }
  }

  metrics->unused += metrics->vuln2_w_prot -
      ( (metrics->vuln2)/3 - (metrics->vuln2-metrics->vuln2_w_prot)/3 );
  metrics->unused += metrics->vuln1_w_prot -
      ( (metrics->vuln1)/2 - (metrics->vuln1-metrics->vuln1_w_prot)/2 );

  metrics->unused += (metrics->safe_op2/2) * 3;
  metrics->unused += (metrics->safe_op1/2) * 2;
  metrics->unused += (metrics->safe_op0/2) * 1;

  *opp_moves = (metrics->empty - (*moves*2) - metrics->unused)/2;
  *opp_moves_real = metrics->opp_real - metrics->prot;
}

void score_board_curr_player(
    const Board& board, int* moves, int* opp_moves, int* opp_moves_real) {
  ScoreMetrics metrics;
  calculate_score_metrics(board, &metrics);

  // We have the next move so...
  if (metrics.prot % 2 == 1) {
    metrics.prot--;
    metrics.safe += 2;
    metrics.opp_real -= 2;
  } else if (metrics.vuln2 % 3 != 0) {
    metrics.vuln2--;
    metrics.safe++;
    if (metrics.vuln2_w_prot > metrics.vuln2) metrics.vuln2_w_prot--;
  } else if (metrics.vuln1 % 2 != 0) {
    metrics.vuln1--;
    metrics.safe++;
    if (metrics.vuln1_w_prot > metrics.vuln1) metrics.vuln1_w_prot--;
  } else if (metrics.safe_op2 % 2 != 0) {
    metrics.safe_op2--;
    metrics.unused += 3;
  } else if (metrics.safe_op1 % 2 != 0) {
    metrics.safe_op1--;
    metrics.unused += 2;
  } else if (metrics.safe_op0 % 2 != 0) {
    metrics.safe_op0--;
    metrics.unused += 1;
  } else if (metrics.vuln2 > 0) {
    metrics.vuln2--;
    metrics.safe++;
    if (metrics.vuln2_w_prot > metrics.vuln2) metrics.vuln2_w_prot--;
  } else if (metrics.vuln1 > 0) {
    metrics.vuln1--;
    metrics.safe++;
    if (metrics.vuln1_w_prot > metrics.vuln1) metrics.vuln1_w_prot--;
  } else if (metrics.prot > 0) {
    metrics.prot--;
    metrics.safe += 2;
  }

  calculate_move_bounds(&metrics, moves, opp_moves, opp_moves_real);
}

void score_board_next_player(
    const Board& board, int* moves, int* opp_moves, int* opp_moves_real) {
  ScoreMetrics metrics;
  calculate_score_metrics(board, &metrics);
  calculate_move_bounds(&metrics, moves, opp_moves, opp_moves_real);
}

void score_board_get_metrics(const Board& board, proto::ScoreMetrics* metrics) {
  ScoreMetrics score;
  calculate_score_metrics(board, &score);

  metrics->set_safe(score.safe);
  metrics->set_opp_real(score.opp_real);
  metrics->set_empty(score.empty);
  metrics->set_prot(score.prot);
  metrics->set_vuln2(score.vuln2);
  metrics->set_vuln2_w_prot(score.vuln2_w_prot);
  metrics->set_vuln1(score.vuln1);
  metrics->set_vuln1_w_prot(score.vuln1_w_prot);
  metrics->set_unused(score.unused);
  metrics->set_safe_op2(score.safe_op2);
  metrics->set_safe_op1(score.safe_op1);
  metrics->set_safe_op0(score.safe_op0);
}

}  // namespace obsequi
