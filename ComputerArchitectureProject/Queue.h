#ifndef QUEUE_H_
#define QUEUE_H_

#include "FileParser.h"

typedef struct _node 
{
	INT32 data1;
	INT32 data2;
	struct _node* next;
	
} NODE, *PNODE;

typedef struct _queue 
{
	UINT32	capacity;
	
	PNODE	head;
	PNODE	tail;
	UINT32	size;
	
} QUEUE, *PQUEUE;

STATUS	Queue_Create(PQUEUE* ppQueue, UINT32 capacity);
STATUS	Queue_Destroy(PQUEUE pQueue);
STATUS	Queue_Enqueue(PQUEUE pQueue, INT32 data1, INT32 data2);
STATUS	Queue_Peek(PQUEUE pQueue, PINT32 pData1, PINT32 pData2);
STATUS	Queue_Dequeue(PQUEUE pQueue, PINT32 pData1, PINT32 pData2);
VOID Queue_Print(PQUEUE pQueue);


#endif //QUEUE_H_