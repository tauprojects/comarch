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
		
		pQueue = malloc(sizeof(QUEUE));
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
			free(cur);
			cur = next;
		}
		
		free(pQueue);
		
	} while(FALSE);
	
	return status;
}

STATUS	Queue_Enqueue(PQUEUE pQueue, INT32 data1, INT32 data2)
{
	STATUS status = STATUS_SUCCESS;
	PNODE pNode = NULL;
	do 
	{
		if(!pQueue)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}
		
		if(pQueue->size == pQueue->capacity)
		{
			status = STATUS_QUEUE_FULL;
			break;
		}
		
		pNode = malloc(sizeof(NODE));
		if(!pNode)
		{
			status = STATUS_MEMORY_FAIL;
			break;
		}
		
		pNode->data1 = data1;
		pNode->data2 = data2;
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

STATUS	Queue_Peek(PQUEUE pQueue, PINT32 pData1, PINT32 pData2)
{
	STATUS 	status = STATUS_SUCCESS;
	
	do 
	{
		if(!pQueue || !pData1 || !pData2)
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
		
		*pData1 = pQueue->head->data1;
		*pData2 = pQueue->head->data2;
	
	} while(FALSE);
	
	return status;
}

STATUS	Queue_Dequeue(PQUEUE pQueue, PINT32 pData1, PINT32 pData2)
{
	STATUS 	status = STATUS_SUCCESS;
	PNODE	next = NULL;
	
	do 
	{
		status = Queue_Peek(pQueue, pData1, pData2);
		
		if(status != STATUS_SUCCESS)
		{
			break;
		}
		
		next = pQueue->head->next;
		free(pQueue->head);
		
		pQueue->head = next;
		pQueue->size--;

		
	} while(FALSE);
	
	return status;
}

VOID Queue_Print(PQUEUE pQueue)
{
	INT32 index = 0;
	PNODE cur = NULL;

	do
	{
		if(!pQueue)
		{
			printf("[Queue]: pQueue is NULL\n");
			break;
		}

		printf("Queue - Capacity <%d>\n",pQueue->capacity);

		if(pQueue->size == 0)
		{
			printf("\tQueue is EMPTY\n");
			break;
		}
		
		cur = pQueue->head;

		while(cur)
		{
			printf("\t%d. (%d, %08x)\n",index,cur->data1,cur->data2);
			index++;
			cur = cur->next;
		}
		printf("Size: %d",pQueue->size);
		printf("\nHead: (%d, %08x)\n",pQueue->head->data1,pQueue->head->data2);
		printf("Tail: (%d, %08x)\n",pQueue->tail->data1,pQueue->tail->data2);

	} while(FALSE);
}
