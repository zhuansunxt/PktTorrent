/**
 * @file test_queue.c
 * @brief Test driver for queue.[h|c]
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../lib/queue.h"

int main(int argc, char** argv) {
  char *q1 = "q1";
  char *q2 = "q2";
  char *q3 = "q3";
  int *i1 = (int*)malloc(sizeof(int));
  *i1 = 1;

  queue *q = queue_new();

  /* test enqueue */
  enqueue(q, q1);
  assert(q->size == 1);
  enqueue(q, q2);
  assert(q->size == 2);
  enqueue(q, q3);
  assert(q->size == 3);

  /* test dequeue */
  assert(strcmp((char*)dequeue(q), q1) == 0);
  assert(q->size == 2);
  assert(strcmp((char*)dequeue(q), q2) == 0);
  assert(q->size == 1);
  assert(strcmp((char*)dequeue(q), q3) == 0);
  assert(q->size == 0);

  /* test int type */
  enqueue(q, i1);
  assert(*((int*)dequeue(q)) == 1);

  /* test queue_free */
  queue_free(q);

  printf("%s succeeds!\n", argv[0]);
  return 0;
}
