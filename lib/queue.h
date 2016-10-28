/**
 * @file queue.h
 * @brief queue data structure.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#ifndef PACTORRENT_QUEUE_H
#define PACTORRENT_QUEUE_H

#include <stdlib.h>

typedef struct qnode_s {
  struct node *next;
  void *data;
} qnode_t;

typedef struct queue_s {
  size_t size;
  qnode_t *front;
  qnode_t *end;
}queue;

queue* queue_new();
void enqueue(queue* q, void *data);
void* dequeue(queue* q);
void* peek(queue* q);
void queue_free(queue *q);

#endif //PACTORRENT_QUEUE_H
