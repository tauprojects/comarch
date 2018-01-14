#include <stdio.h>
#include <stdlib.h>
#include "Queue.h"


STATUS	Queue_Create(PQUEUE* ppQueue, UINT32 capacity)
{
	STATUS status = STATUS_SUCCESS;
	PQUEUE pQueue = NULL;
	
	do 
	{
		if(!capacity || !ppQueue)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}
		
		pQueue = safeMalloc(sizeof(QUEUE));
		if(!pQueue)
		{
			status = STATUS_MEMORY_FAIL;
			break;
		}
		
		pQueue->capacity = capacity;
		pQueue->size = 0;
		pQueue->head = NULL;
		pQueue->tail = NULL;
		
		*ppQueue = pQueue;
		
	} while(FALSE);
	
	return status;
}

STATUS	Queue_Destroy(PQUEUE pQueue)
{
	STATUS	status = STATUS_SUCCESS;
	PNODE	cur = NULL;
	PNODE	next = NULL;
	do 
	{
		if(!pQueue)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}
		
		cur = pQueue->head;
		
		while(cur)
		{
			next = cur->next;

			//clear instruction context if its not NULL
			if (cur->pInstCtx)
				safeFree(cur->pInstCtx);

			safeFree(cur);
			cur = next;
		}
		
		safeFree(pQueue);
		
	} while(FALSE);
	
	return status;
}

STATUS	Queue_Enqueue(PQUEUE pQueue, PInstCtx pInstCtx)
{
	STATUS status = STATUS_SUCCESS;
	PNODE pNode = NULL;

	do 
	{
		if(!pQueue || !pInstCtx)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}
		
		if(pQueue->size == pQueue->capacity)
		{
			status = STATUS_QUEUE_FULL;
			break;
		}
		
		pNode = safeMalloc(sizeof(NODE));
		if(!pNode)
		{
			status = STATUS_MEMORY_FAIL;
			break;
		}

		pNode->pInstCtx = pInstCtx;
		pNode->next = NULL;
		
		if(pQueue->size == 0)
		{
			pQueue->head = pNode;
			pQueue->tail = pNode;
		}
		else
		{
			pQueue->tail->next = pNode;
			pQueue->tail = pNode;
		}
		
		pQueue->size++;
		
	} while(FALSE);
	
	return status;
}

STATUS	Queue_IsFull(PQUEUE pQueue, PBOOL pbIsFull)
{
	STATUS status = STATUS_SUCCESS;

	do
	{
		if (!pQueue || !pbIsFull)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}
		
		*pbIsFull = (pQueue->capacity == pQueue->size);

	} while (FALSE);

	return status;
}

STATUS	Queue_Peek(PQUEUE pQueue, PPInstCtx ppInstCtx)
{
	STATUS 	status = STATUS_SUCCESS;
	
	do 
	{
		if(!pQueue || !ppInstCtx)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}
		
		if(pQueue->size == 0)
		{
			status = STATUS_QUEUE_EMPTY;
			break;
		}
		
		if(!pQueue->head)
		{
			status = STATUS_INVALID_QUEUE;
			break;
		}
		
		*ppInstCtx = pQueue->head->pInstCtx;
	
	} while(FALSE);
	
	return status;
}

STATUS	Queue_Dequeue(PQUEUE pQueue, PPInstCtx ppInstCtx)
{
	STATUS 	status = STATUS_SUCCESS;
	PNODE	next = NULL;
	
	do 
	{
		status = Queue_Peek(pQueue, ppInstCtx);
		
		if(status != STATUS_SUCCESS)
		{
			break;
		}
		
		next = pQueue->head->next;
		safeFree(pQueue->head);
		
		pQueue->head = next;
		pQueue->size--;
		
	} while(FALSE);
	
	return status;
}

