#include "lastbit.h"

// You can easily test this by running:
// gcc `./cppflags` -DTEST lastbit.c && ./a.out

// Last bit
s32bit lastbit8[256];
s32bit lastbit16[65536];

void
__attribute__ ((constructor))
init_lastbit()
{
  s32bit i;

  lastbit16[0] = 100;
  for(i = 1; i < 65536; i++){
    if(i&NTH_BIT(0)) {lastbit16[i] = 0; continue;}
    if(i&NTH_BIT(1)) {lastbit16[i] = 1; continue;}
    if(i&NTH_BIT(2)) {lastbit16[i] = 2; continue;}
    if(i&NTH_BIT(3)) {lastbit16[i] = 3; continue;}
    if(i&NTH_BIT(4)) {lastbit16[i] = 4; continue;}
    if(i&NTH_BIT(5)) {lastbit16[i] = 5; continue;}
    if(i&NTH_BIT(6)) {lastbit16[i] = 6; continue;}
    if(i&NTH_BIT(7)) {lastbit16[i] = 7; continue;}
    if(i&NTH_BIT(8)) {lastbit16[i] = 8; continue;}
    if(i&NTH_BIT(9)) {lastbit16[i] = 9; continue;}
    if(i&NTH_BIT(10)) {lastbit16[i] = 10; continue;}
    if(i&NTH_BIT(11)) {lastbit16[i] = 11; continue;}
    if(i&NTH_BIT(12)) {lastbit16[i] = 12; continue;}
    if(i&NTH_BIT(13)) {lastbit16[i] = 13; continue;}
    if(i&NTH_BIT(14)) {lastbit16[i] = 14; continue;}
    if(i&NTH_BIT(15)) {lastbit16[i] = 15; continue;}
  }

  lastbit8[0] = 100;
  for(i = 1; i < 256; i++){
    if(i&NTH_BIT(0)) {lastbit8[i] = 0; continue;}
    if(i&NTH_BIT(1)) {lastbit8[i] = 1; continue;}
    if(i&NTH_BIT(2)) {lastbit8[i] = 2; continue;}
    if(i&NTH_BIT(3)) {lastbit8[i] = 3; continue;}
    if(i&NTH_BIT(4)) {lastbit8[i] = 4; continue;}
    if(i&NTH_BIT(5)) {lastbit8[i] = 5; continue;}
    if(i&NTH_BIT(6)) {lastbit8[i] = 6; continue;}
    if(i&NTH_BIT(7)) {lastbit8[i] = 7; continue;}
  }
}

#ifdef TEST
#include <time.h>

void
compare_methods(u32bit x) {
  int b8 = lastbit32_8bitmap(x);
  int b16 = lastbit32_16bitmap(x);
  int bn = lastbit32_gccbuiltin(x);

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
  compare_methods(0);
  compare_methods(~0);

  int i;
  for (i = 0; i < (1<<16); i++) {
    compare_methods(i);
  }

  // Compare speeds.
  clock_t start, end;
  int a = 0;

  int max = 1000000000;

  start = clock();
  a = 0;
  for (i = 0; i < max; i++) {
    a += lastbit32_8bitmap(i);
  }
  end = clock();
  printf("lastbit32_8 : %lu CLOCKS\n", (end - start));

  start = clock();
  for (i = 0; i < max; i++) {
    a += lastbit32_16bitmap(i);
  }
  end = clock();
  printf("lastbit32_16: %lu CLOCKS\n", (end - start));

  start = clock();
  for (i = 0; i < max; i++) {
    a += lastbit32_gccbuiltin(i);
  }
  end = clock();
  printf("lastbit32_bu: %lu CLOCKS\n", (end - start));
  printf("%d\n", a);

  return 0;
}
#endif
