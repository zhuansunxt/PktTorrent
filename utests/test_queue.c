/**
 * @file test_queue.c
 * @brief Test driver for queue.[h|c]
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../lib/queue.h"
#include "../packet/packet.h"
#include "../core/download_handler.h"
#include "../utilities/sha.h"

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
  assert(strcmp((const char*)dequeue(q), q1) == 0);
  assert(q->size == 2);
  assert(strcmp((const char*)dequeue(q), q2) == 0);
  assert(q->size == 1);
  assert(strcmp((const char*)dequeue(q), q3) == 0);
  assert(q->size == 0);

  /* test int type */
  enqueue(q, i1);
  assert(*((int*)dequeue(q)) == 1);

  /* test packet */
  char *chunk_hash_1 = "1dddf0fa5abd782bb3e1a661ea307368c522aefd";
  char *chunk_hash_2 = "1dddf0fa5abd782bb3e1a661ea307368c522aefe";
  packet_t *get_packet_1 = build_get_packet(chunk_hash_1);
  packet_t *get_packet_2 = build_get_packet(chunk_hash_2);
  enqueue(q, get_packet_1);
  enqueue(q, get_packet_2);
  assert(q->size == 2);

  packet_t *test;
  char chunk[HASH_STR_LEN];
  test = (packet_t*)dequeue(q);
  hex2ascii(test->payload, SHA1_HASH_SIZE, chunk);
  assert(!strcmp(chunk_hash_1, chunk));
  assert(q->size == 1);
  test = (packet_t*)dequeue(q);
  hex2ascii(test->payload, SHA1_HASH_SIZE, chunk);
  assert(!strcmp(chunk_hash_2, chunk));
  assert(q->size == 0);

  pkt_free(get_packet_1);
  pkt_free(get_packet_2);

  /* test queue_free */
  queue_free(q);
  free(i1);


  printf("%s succeeds!\n", argv[0]);
  return 0;
}
