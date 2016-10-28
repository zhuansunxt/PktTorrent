/**
 * @file queue.c
 * @brief Implementation for queue data structure.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */
#include "queue.h"

/**
 * Initialize a new queue instance.
 * @return newly allocated queue.
 */
queue *queue_new() {
  queue *q = (queue*)malloc(sizeof(queue));
  q->front = NULL;
  q->end = NULL;
  q->size = 0;
  return q;
}

/**
 * Append data to end of the queue.
 * @param q queue instance.
 * @param data data to be enqueueed.
 */
void enqueue(queue* q, void *data) {
  qnode_t *qnode = (qnode_t*)malloc(sizeof(qnode_t));
  qnode->data = data;
  qnode->next = NULL;

  if (q->size == 0 && q->front == NULL) {
    q->front = qnode;
    q->end = qnode;
  } else {
    q->end->next = qnode;
    q->end = qnode;
  }

  q->size++;
}

/**
 * Poll data from the front of the queue.
 * @param q queue instance.
 * @return data contained in the front node in queue.
 */
void* dequeue(queue* q) {
  if (q->size == 0 && q->front == NULL) {
    return NULL;
  }

  qnode_t *head = q->front;
  void *data = q->front->data;

  if (q->size == 0 && (q->front==q->end)) {
    q->front = NULL;
    q->end = NULL;
  } else {
    q->front = q->front->next;
  }

  q->size--;
  free(head);
  return data;
}


void* peek(queue* q) {
  if (q->size == 0) {
    return NULL;
  } else {
    return q->front->data;
  }
}

void queue_free(queue *q) {
  void *dummy;
  do {
    dummy = dequeue(q);
  } while (dummy != NULL);
}

