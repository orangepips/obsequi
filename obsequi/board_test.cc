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
}  // namespace obsequi

TEST(MoveOrdering) {
  Move x = {1,1};
  Move y = {4,3};
  Board b(6,8);
  b.ApplyMove(y);
  b.ApplyMove(x);
  Board c(6,8);
  c.ApplyMove(x);
  c.ApplyMove(y);

  const HashKeys keys1 = b.GetHashKeys();
  const HashKeys keys2 = c.GetHashKeys();

  for (int i = 0; i < 4; i++) {
    assert(keys1.mod[i].code == keys2.mod[i].code);
    assert(keys1.mod[i].key[0] == keys2.mod[i].key[0]);
    assert(keys1.mod[i].key[1] == keys2.mod[i].key[1]);
  }

  // c.Print();
}

TEST(MovePlayer) {
  // Horizontal
  Move h1 = {2,1};
  Move h2 = {3,1};
  Board b(6,8);
  b.ApplyMove(h1);
  b.ApplyMove(h2);

  Move v1 = {1,2};
  Move v2 = {2,2};
  Board c(6,8);
  c.GetOpponent()->ApplyMove(v1);
  c.GetOpponent()->ApplyMove(v2);

  // b.Print();
  // printf("\n");
  // printf("\n");
  // c.Print();

  const HashKeys keys1 = b.GetHashKeys();
  const HashKeys keys2 = c.GetHashKeys();

  for (int i = 0; i < 4; i++) {
    // printf("Iter: %d\n", i);
    // printf("code: 0x%X   key: 0x%lX 0x%lX\n", keys1.mod[i].code,
    //        keys1.mod[i].key[0], keys1.mod[i].key[1]);
    // printf("code: 0x%X   key: 0x%lX 0x%lX\n", keys2.mod[i].code,
    //        keys2.mod[i].key[0], keys2.mod[i].key[1]);
    assert(keys1.mod[i].code == keys2.mod[i].code);
    assert(keys1.mod[i].key[0] == keys2.mod[i].key[0]);
    assert(keys1.mod[i].key[1] == keys2.mod[i].key[1]);
  }
}
