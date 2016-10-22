/**
 * @file commons.h
 * @brief Implementation of common helpers.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include "commons.h"

void console_log(const char *fmt, ...){
  va_list(args);
  va_start(args, fmt);
  vprintf(fmt, args);
  /* Auto new line */
  printf("\n");
  va_end(args);
}