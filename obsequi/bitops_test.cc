#include "bitops.h"

#include "test.h"

#include <assert.h>
#include <time.h>

using namespace obsequi;
using namespace obsequi::internal;

void compare_lastbit32(u32bit mask, int expected) {
  int b8 = lastbit32_8bitmap(mask);
  int b16 = lastbit32_16bitmap(mask);
  int bn = lastbit32_gccbuiltin(mask);

  // printf("FAILED expected-b8: %d %d %d\n", mask, expected, b8);
  assert(b8 == expected);
  assert(b16 == expected);
  assert(bn == expected);
}

TEST(lastbit_validate) {
  compare_lastbit32(0, 100);
  compare_lastbit32(~0, 0);
  
  for (int i = 0; i < (1<<16); i++) {
    compare_lastbit32(i, lastbit32_8bitmap(i));
  }
}

NO_TEST(lastbit_performance) {
  const int max = 1000000000;

  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += lastbit32_8bitmap(i);
    }
    clock_t end = clock();
    printf("lastbit32_8 : %lu CLOCKS, (%d)\n", (end - start), a);
  }
  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += lastbit32_16bitmap(i);
    }
    clock_t end = clock();
    printf("lastbit32_16 : %lu CLOCKS, (%d)\n", (end - start), a);
  }
  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += lastbit32_gccbuiltin(i);
    }
    clock_t end = clock();
    printf("lastbit32_bu : %lu CLOCKS, (%d)\n", (end - start), a);
  }
}


void compare_countbits32(u32bit mask, int expected) {
  int b8 = countbits32_8bitmap(mask);
  int b16 = countbits32_16bitmap(mask);
  int bn = countbits32_gccbuiltin(mask);

  // printf("FAILED expected-b8: %d %d %d\n", mask, expected, b8);
  assert(b8 == expected);
  assert(b16 == expected);
  assert(bn == expected);
}

TEST(countbits_validate) {
  compare_countbits32(0, 0);
  compare_countbits32(~0, 32);
  
  for (int i = 0; i < (1<<16); i++) {
    compare_countbits32(i, countbits32_gccbuiltin(i));
  }
}

NO_TEST(countbits_performance) {
  const int max = 1000000000;

  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += countbits32_8bitmap(i);
    }
    clock_t end = clock();
    printf("countbits32_8 : %lu CLOCKS, (%d)\n", (end - start), a);
  }
  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += countbits32_16bitmap(i);
    }
    clock_t end = clock();
    printf("countbits32_16 : %lu CLOCKS, (%d)\n", (end - start), a);
  }
  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += countbits32_gccbuiltin(i);
    }
    clock_t end = clock();
    printf("countbits32_bu : %lu CLOCKS, (%d)\n", (end - start), a);
  }
}


static inline int countmoves32_calc(u32bit mask) {
  int count = 0;
  u32bit tmp;

  while(mask){
    tmp = (mask&-mask);           // least sig bit of m
    mask &= ~(tmp | (tmp << 1));  // remove bit and next bit.
    count++;
  }
  return count;
}

void compare_countmoves(u32bit mask, int expected) {
  int b16 = countmoves32_16bitmap(mask);
  int bn = countmoves32_calc(mask);

  if (!(mask & ~0xffff)) {
    int b8 = countmoves16_16bitmap(mask);
    assert(b8 == expected);
  }
  //printf("Expected-b16: 0x%x %d %d\n", mask, expected, b16);
  assert(b16 == expected);
  assert(bn == expected);
}

TEST(countmoves_validate) {
  compare_countmoves(0, 0);
  compare_countmoves(~0, 16);

  for (u32bit mask = 0x1; mask; mask <<= 1) {
    compare_countmoves(mask, countmoves32_calc(mask));
  }
  for (u32bit mask = 0x3; mask; mask <<= 1) {
    compare_countmoves(mask, countmoves32_calc(mask));
  }
  for (u32bit mask = 0x7; mask; mask <<= 1) {
    compare_countmoves(mask, countmoves32_calc(mask));
  }
  for (u32bit mask = 0xF; mask; mask <<= 1) {
    compare_countmoves(mask, countmoves32_calc(mask));
  }
  for (u32bit mask = 0x1F; mask; mask <<= 1) {
    compare_countmoves(mask, countmoves32_calc(mask));
  }
}

NO_TEST(countmoves_performance) {
  const int max = 1000000000;

  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += countmoves16_16bitmap(i & 0xFFFF);
    }
    clock_t end = clock();
    printf("countmoves16_16bitmap : %lu CLOCKS, (%d)\n", (end - start), a);
  }
  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += countmoves32_16bitmap(i);
    }
    clock_t end = clock();
    printf("countmoves32_16 : %lu CLOCKS, (%d)\n", (end - start), a);
  }
  {
    clock_t start = clock();
    int a = 0;
    for (int i = 0; i < max; i++) {
      a += countmoves32_calc(i);
    }
    clock_t end = clock();
    printf("countbits32_bu : %lu CLOCKS, (%d)\n", (end - start), a);
  }
}

