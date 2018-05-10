// Simple implementation of FIFO queue of integers, using doubly linked list.

/* doubly linked list node */
typedef struct qnode *QNode;
struct qnode {
  void *val;
  QNode next;
  QNode prev;
};

/* A Queue is a pointer to a structure. 
*/
typedef struct queue *Queue;
struct queue {
  QNode front;
  QNode back;
  int size;
};

/* make an empty queue */
Queue makeQueue();

/* destroy the queue */
void freeQueue(Queue q);

/* enqueue (assuming qp non-null) */
void enqueue(Queue qp, void *val);

/* 1 if queue has elements, else 0 (assuming qp non-null) */
int nonempty(Queue qp);

/* dequeue, assuming qp non-null and queue nonempty */
void *dequeue(Queue qp);
