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

/*
 * t1 will be ensured to be later than t2
 * return millisecond
 */
long get_time_diff(struct timeval *t1, struct timeval *t2) {
  assert(t1->tv_sec >= t2->tv_sec && t1->tv_usec >= t2->tv_usec);

  long diff = (t1->tv_sec - t2->tv_sec) * 1000L +
          (t1->tv_usec - t2->tv_usec) / 1000L;

  return diff;
}