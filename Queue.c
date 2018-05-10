#include <stdlib.h>
#include <stdio.h>

#include "Queue.h"


/* make an empty queue */
Queue makeQueue() {
  Queue q = (Queue) malloc(sizeof(struct queue));
  q->front = q->back = NULL;
  q->size = 0;
  return q;
}

void freeQueue(Queue q) {
  // First empty the queue
  while (nonempty(q)) {
    dequeue(q);
  }

  q->size = 0;
  // Then free the memory
  free(q);
}


void enqueue(Queue qp, void *val) {
  QNode n = (QNode) malloc(sizeof(struct qnode));
  n->val = val;
  n->next = qp->front;
  n->prev = NULL;
  if (qp->front != NULL)
    qp->front->prev = n;
  else
    qp->back = n;

  qp->front = n;
  qp->size++;
}


int nonempty(Queue qp) {
  return qp->front != NULL;
}


void *dequeue(Queue qp) {

  QNode back = qp->back;
  void *result = back->val;
  qp->back = back->prev;
  if (back->prev != NULL)
    back->prev->next = NULL;
  else
    qp->front = NULL;
  back->val = 0;
  back->prev = NULL;
  back->next = NULL;
  free(back);

  qp->size--;
  return result;
}
