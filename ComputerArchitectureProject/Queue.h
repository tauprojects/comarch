#ifndef QUEUE_H_
#define QUEUE_H_

#include "mainDefs.h"

typedef struct _node_inst
{
	PInstCtx			pInstCtx;
	struct _node_inst*	next;

} NODE, *PNODE;

typedef struct _queue 
{
	UINT32	capacity;
	
	PNODE	head;
	PNODE	tail;
	UINT32	size;
	
} QUEUE, *PQUEUE;

STATUS	Queue_Create	(PQUEUE* ppQueue, UINT32 capacity);
STATUS	Queue_Destroy	(PQUEUE pQueue);
STATUS	Queue_Enqueue	(PQUEUE pQueue, PInstCtx pInstCtx);
STATUS	Queue_Peek		(PQUEUE pQueue, PPInstCtx ppInstCtx);
STATUS	Queue_Dequeue	(PQUEUE pQueue, PPInstCtx ppInstCtx);
VOID 	Queue_Print		(PQUEUE pQueue);
STATUS	Queue_IsFull	(PQUEUE pQueue, PBOOL pbIsFull);


#endif //QUEUE_H_