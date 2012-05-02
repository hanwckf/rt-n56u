#ifndef _QUEUE_H_
#define _QUEUE_H_ 1

struct queue_e
{
    struct queue_e *next;
};

typedef struct queue_e *q_ptr;

#define  queue_e q_ptr

typedef struct queue_s
{
   queue_e head;
   queue_e tail;
   int   len;
   int   max_len;
   int   min_len;
} QUEUE, *PQUEUE;

void *DeQueue( QUEUE *q);
void EnQueue( QUEUE *q, void *e);
void ResetQueue( QUEUE *q);

#endif  /* _QUEUE_H_ */

