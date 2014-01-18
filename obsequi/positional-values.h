// Count the number of bits set

#ifndef OBSEQUI_POSITIONAL_VALUES_H
#define OBSEQUI_POSITIONAL_VALUES_H

namespace obsequi {

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

  // Disallow copy and assign.
  PositionalValues(const PositionalValues&);
  void operator=(const PositionalValues&);
};

}  // namespace obsequi
#endif  // OBSEQUI_POSITIONAL_VALUES_H
