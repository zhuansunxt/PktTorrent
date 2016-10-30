/**
 * @file test_diff_time.c
 * @brief Testing for get_diff_time utility function.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include "../utilities/commons.h"
#include <assert.h>
#include <unistd.h>

int main(int argc, char** argv) {
  struct timeval t1;
  struct timeval t2;
  unsigned int time_interval = 2;

  gettimeofday(&t1, NULL);
  sleep(time_interval);
  gettimeofday(&t2, NULL);

  long diff = get_time_diff(&t2, &t1);
  assert(diff >= time_interval*1000-50 && diff <= time_interval*1000+50);

  printf("%s succeeds!\n", argv[0]);
  return 0;
}


