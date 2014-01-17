#include "bitops.h"

namespace obsequi {

namespace internal {
int lastbit8[256];
int lastbit16[65536];

void __attribute__ ((constructor)) init_lastbit() {
  lastbit16[0] = 100;
  for (int i = 1; i < 65536; i++){
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
  for (int i = 1; i < 256; i++){
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


int countbits8[256];
int countbits16[65536];

void __attribute__ ((constructor)) init_countbits() {
  countbits8[0] = 0;
  for (int i = 1; i < 256; i++){
    countbits8[i] = (i & 1) + countbits8[i >> 1];
  }

  countbits16[0] = 0;
  for (int i = 1; i < 65536; i++){
    countbits16[i] = (i & 1) + countbits16[i >> 1];
  }
}


int move_table16[65536];

void __attribute__ ((constructor)) init_movetable() {
  u32bit i = 0;

  while(i < 65536){
    u32bit mask = i;
    u32bit count = 0;
    u32bit tmp = 0;

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

}  // namespace obsequi::internal
}  // namespace obsequi
