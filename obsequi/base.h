#ifndef OBSEQUI_BASE_H
#define OBSEQUI_BASE_H

#include <cinttypes>

// Some base types for when you want a very specific size.
typedef uint8_t u8bit;
typedef uint16_t u16bit;
typedef uint32_t u32bit;
typedef uint64_t u64bit;

typedef int16_t s16bit;
typedef int32_t s32bit;

// I suppose I could use std::max/min
#define MAX_TWO(a,b) (((a) > (b)) ? (a) : (b))
#define MIN_TWO(a,b) (((a) > (b)) ? (b) : (a))

// Simple bit macros
#define ALL_BITS_32      0xFFFFFFFF
#define ALL_BITS_64      0xFFFFFFFFFFFFFFFFul
#define NTH_BIT(i)       ( 1ul << (i) )

// Limits
#define MAX_MOVES 256
#define MAX_ROWS 32
#define MAX_SEARCH_DEPTH 80

// Format a number with commas: 1,000,000
const char* u64bit_to_string(u64bit val, char* buffer);

// Print a message, with file name and line number, and exit.
#define fatal_error(err_num, format, rest...) \
         (_fatal_error_aux(__FILE__, __LINE__, (err_num), format, ## rest ))

void _fatal_error_aux(const char *file, const s32bit line,
                      const s32bit err_num, const char *format, ... )
     __attribute__ ((format (printf, 4, 5)));

#endif  // OBSEQUI_BASE_H
