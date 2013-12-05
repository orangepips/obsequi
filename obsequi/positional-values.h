// Count the number of bits set

#ifndef POSITIONAL_VALUES_H
#define POSITIONAL_VALUES_H

#include <stdio.h>

class PositionalValues {
 public:
  PositionalValues(int num_rows, int num_cols);
  ~PositionalValues() {}

  int GetValue(int row, int col) const {
    return positional_values_[row][col];
  }
  
  void Print() const;

 private:
  void SetValue(int num_rows, int num_cols, int row, int col, int value);

  int positional_values_[32][32];
};

/*
#ifdef DYNAMIC_POSITION_VALUES
void    init_move_value     ();
s32bit  set_move_value      (Move movelist, s32bit player);
void    unset_move_value    (Move movelist, s32bit player);
#else
void    set_position_values ();
#endif
*/

#endif // POSITIONAL_VALUES_H
