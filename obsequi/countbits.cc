#include "countbits.h"

// You can easily test this by running:
// gcc `./cppflags` -DTEST countbits.c && ./a.out

// Count bits
s32bit countbits8[256];
s32bit countbits16[65536];

void
__attribute__ ((constructor))
init_countbits()
{
  s32bit i;

  countbits8[0] = 0;
  for(i = 1; i < 256; i++){
    countbits8[i] = (i & 1) + countbits8[i >> 1];
  }

  countbits16[0] = 0;
  for(i = 1; i < 65536; i++){
    countbits16[i] = (i & 1) + countbits16[i >> 1];
  }
}

#ifdef TEST
#include <time.h>

void
compare_counts(u32bit x) {
  int b8 = countbits32_8bitmap(x);
  int b16 = countbits32_16bitmap(x);
  int bn = countbits32_gccbuiltin(x);

  if (b8 != b16) {
    printf("FAILED b8-b16: %d %d %d\n", x, b8, b16);
    exit(1);
  }

  if (b8 != bn) {
    printf("FAILED b8-bn: %d %d %d\n", x, b8, bn);
    exit(1);
  }
}

int
main() {
  compare_counts(0);
  compare_counts(~0);

  int i;
  for (i = 0; i < (1<<16); i++) {
    compare_counts(i);
  }

  // Compare speeds.
  clock_t start, end;
  int a = 0;

  int max = 1000000000;

  start = clock();
  a = 0;
  for (i = 0; i < max; i++) {
    a += countbits32_8bitmap(i);
  }
  end = clock();
  printf("countbits32_8 : %lu CLOCKS\n", (end - start));

  start = clock();
  for (i = 0; i < max; i++) {
    a += countbits32_16bitmap(i);
  }
  end = clock();
  printf("countbits32_16: %lu CLOCKS\n", (end - start));

  start = clock();
  for (i = 0; i < max; i++) {
    a += countbits32_gccbuiltin(i);
  }
  end = clock();
  printf("countbits32_bu: %lu CLOCKS\n", (end - start));
  printf("%d\n", a);

  return 0;
}
#endif
