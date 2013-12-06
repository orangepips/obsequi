#include "countmoves.h"

// You can easily test this by running:
// gcc `./cppflags` -DTEST countmoves.c && ./a.out

s32bit move_table16[65536];

void
__attribute__ ((constructor))
init_movetable() {
  u32bit i = 0, mask, tmp = 0, count;

  while(i < 65536){
    mask = i;
    count = 0;

    while(mask){
      tmp = (mask&-mask);           // least sig bit of m
      mask &= ~(tmp | (tmp << 1));  // remove bit and next bit.
      count++;
    }

    // Set a special flag that helps us merge two 16 bit masks together.
    if(tmp & 0x8000) count |= ERASE_NEXT_BIT;
    move_table16[i] = count;
    i++;
  }
}


#ifdef TEST
#include <time.h>

void
compare_methods(u32bit x) {
  int b16 = countmoves32_16bitmap(x);
  int bn = countmoves32_calc(x);

  if (!(x & ~0xffff)) {
    int b8 = countmoves16_16bitmap(x);
    if (b8 != bn) {
      printf("FAILED b8-bn: 0x%x %d %d\n", x, b8, bn);
      exit(1);
    }
  }

  if (b16 != bn) {
    printf("FAILED b16-bn: 0x%x %d %d\n", x, b16, bn);
    exit(1);
  }
}

int
main() {
  printf("Starting Test... This can take a while.\n");

  u32bit max = ~0, i;
  for (i = 0; i < max; i++) {
    compare_methods(i);
  }
  compare_methods(~0);

  // Compare speeds.
  clock_t start, end;
  int a = 0;  // Force compiler to not optimize everything away.

  start = clock();
  for (i = 0; i < max; i++) {
    a += countmoves16_16bitmap(i & 0xFFFF);
  }
  end = clock();
  printf("countmoves16_16: %lu CLOCKS\n", (end - start));

  start = clock();
  for (i = 0; i < max; i++) {
    a += countmoves32_16bitmap(i);
  }
  end = clock();
  printf("countmoves32_16: %lu CLOCKS\n", (end - start));

  start = clock();
  for (i = 0; i < max; i++) {
    a += countmoves32_calc(i);
  }
  end = clock();
  printf("countmoves32_bu: %lu CLOCKS\n", (end - start));
  printf("%d\n", a);

  return 0;
}
#endif
