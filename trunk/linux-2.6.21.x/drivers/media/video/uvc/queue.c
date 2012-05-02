#include "queue.h"

void *DeQueue( QUEUE *q)
{
    queue_e   temp = q->head;
    
    if ( temp == 0)
    {
        if (q->len != 0)
            printk( "DeQueue: !!!!ERROR!!!!\n");
        return  (void *)0;
    }
    
    q->head = temp->next;
    temp->next = 0;
    
    if ( q->head == 0)
    {
        q->tail = 0;
    }
    
    q->len--;
    
    if ( q->len < q->min_len)
    {
        q->min_len = q->len;
    }

    return ((void*)temp);
}

void EnQueue( QUEUE *q, void *e)
{
    ((q_ptr)e)->next = 0;

    if ( q->head == 0)
    {
        q->head = (q_ptr)e;
    }
    else
    {
        q->tail->next = (q_ptr)e;
    }
   
    q->tail = (q_ptr)e;
    
    if ( ++(q->len) > q->max_len)
    {
        q->max_len = q->len;
    }
}

void ResetQueue( QUEUE *q)
{
	q->head = 0;
	q->tail = 0;
	q->len = 0;
	q->max_len = 0;
	q->min_len = 9999;
}

