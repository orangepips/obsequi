#include "base.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//########################################################
// This function prints a message to both stderr and
//   to the file ".fatal_error".
//
// If ERR_NUM != 0 then exit is called with the value ERR_NUM.
//########################################################
void _fatal_error_aux(const char *file, const s32bit line,
		      const s32bit err_num, const char *format, ... ) {
  va_list ap;
  static FILE* err_file = NULL;
  const char *msg_type = (err_num == 0) ? "WARNING: " : "ERROR: ";

  // only need to open this if not already open.
  if(err_file == NULL)  err_file = fopen(".fatal_error", "w");
  if(err_file == NULL) fprintf(stderr, "Couldn't open \".fatal_error\".\n");

  if(err_num == 0) {
    // Check size of file, we don't want to end up filling up the whole disk.
    s32bit size = ftell(err_file);
    if(size == -1)
      fatal_error(1, "size == -1.\n");

    else if(size > (2<<24) + 2000 ){
      return;
    }

    else if(size > (2<<24)){
      fprintf(stderr, "Log file getting too large.\n");
      fprintf(err_file, "Log file getting too large.\n");
    }
  }

  // start writing the real message.
  fprintf(stderr, "%s", msg_type);
  fprintf(err_file, "%s", msg_type);

  va_start (ap, format);
  vfprintf (stderr, format, ap);
  vfprintf (err_file, format, ap);
  va_end (ap);

  fprintf(stderr, "> File: %s, Line: %d.\n", file, line);
  fprintf(err_file, "> File: %s, Line: %d.\n", file, line);

  fflush(stderr);
  fflush(err_file);

  if (err_num != 0)  exit(err_num);
}

//########################################################
// Return a string with the printed value of a 64 bit number.
//########################################################
const char* u64bit_to_string(u64bit val, char* buffer) {
  int vals[10];
  int i = 0;

  do {
    vals[i] = val % 1000;
    val = val / 1000;
    i++;
  } while(val != 0);

  int offset = sprintf(buffer, "%d", vals[--i]);

  while(i != 0)
    offset += sprintf(buffer + offset, ",%03d", vals[--i]);

  return buffer;
}
