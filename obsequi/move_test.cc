#include "move.h"

#include "board.h"
#include "test.h"

#include <assert.h>
#include <time.h>
#include <vector>
#include <algorithm>

using namespace obsequi;

namespace obsequi {

bool operator==(const Move& a, const Move& b) {
  return a.array_index == b.array_index &&
    a.mask_index == b.mask_index;
}

bool Find(const MoveList& ml, const Move& m) {
  for (int i = 0; i < ml.Size(); i++) {
    if (ml[i] == m) return true;
  }
  return false;
}
}  // namespace obsequi

// Verify for a given board GenerateAllMoves and GetNext find the same moves.
void CompareGenerators(Board* b) {
  MoveList ml1;
  ml1.GenerateAllMoves(b);

  MoveList ml2;
  const Move* m;
  while ((m = ml2.GetNext(b)) != nullptr) {
    //printf("(%d, %d)\n", m->array_index, m->mask_index);
    assert(Find(ml1, *m));
  }

  //printf("\n\n");
  for (int i = 0; i < ml1.Size(); i++) {
    //printf("(%d, %d)\n", ml1[i].array_index, ml1[i].mask_index);
    assert(Find(ml2, ml1[i]));
  }
}

TEST(BasicMove) {
  Board b(4,4);
  b.SetBlock(1, 0);
  b.SetBlock(1, 1);
  b.SetBlock(1, 2);
  b.SetBlock(1, 3);
  b.SetBlock(3, 1);

  CompareGenerators(&b);
}

TEST(StagedGeneration) {
  Board b(4,4);
  b.SetBlock(2, 1);

  MoveList ml;
  const Move* m;
  int prev = 0;

  // Initial size should always be 0.
  assert(ml.Size() == 0);

  for (int i = 0; i < 10; i++) {
    m = ml.GetNext(&b);
    assert(m);
    if (i == 0) {
      assert(ml.Size() == 1);
    } else if (i < 8) {
      assert(ml.Size() == 8);
    } else {
      assert(ml.Size() == 10);
    }
    if (i != 0) {
      // We may find a test case where this isn't true for the boundary
      // between the first and second stage.
      assert(prev >= m->info);
    }
    prev = m->info;
  }
  m = ml.GetNext(&b);
  assert(!m);
  assert(ml.Size() == 10);
}
