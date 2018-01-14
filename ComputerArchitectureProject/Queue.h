#ifndef QUEUE_H_
#define QUEUE_H_

#include "mainDefs.h"

/* Queue Node - holding only pointer to instruction context */
typedef struct _node_inst
{
	PInstCtx			pInstCtx;
	struct _node_inst*	next;

} NODE, *PNODE;

/* Struct of queue */
typedef struct _queue 
{
	UINT32	capacity; //constant - capacity of queue
	
	PNODE	head;
	PNODE	tail;
	UINT32	size;	//current size of queue
	
} QUEUE, *PQUEUE;

/**
* This function creates a new queue with the size capacity.
* @param ppQueue	-	pointer which dereferenced value will hold the newly created queue.
* @param capacity
*
* @ret #STATUS_INVALID_ARGS if capacity == 0 or ppQueue == NULL, #STATUS_MEMORY_FAIL if allocation fails,
*		otherwise #STATUS_SUCCESS
*/
STATUS	Queue_Create	(PQUEUE* ppQueue, UINT32 capacity);

/**
* This function deallocates a queue, along with its all connected pointers (every element, and in each one,
* every instruction context).
* @param pQueue		-	pointer to the queue
*
* @ret #STATUS_INVALID_ARGS if pQueue == NULL, otherwise #STATUS_SUCCESS
*/
STATUS	Queue_Destroy	(PQUEUE pQueue);


/**
* This function enqueues a new pInstCtx. It allocates a new queue node, and assigns the pInstCtx to it.
* It inserts the node to the tail of the queue, if the queue is not full.
*
* @param pQueue		-	pointer to the queue
* @param pInstCtx	-	instruction context to enqueue
*
* @ret #STATUS_INVALID_ARGS if pQueue == NULL or pInstCtx == NULL, #STATUS_QUEUE_FULL if queue is full,
*	   #STATUS_MEMORY_FAIL if memory allocation fails, otherwise #STATUS_SUCCESS
*/
STATUS	Queue_Enqueue	(PQUEUE pQueue, PInstCtx pInstCtx);

/**
* This function returns a pointer to the instruction in the head node of the queue pQueue.
*
* @param pQueue		-	pointer to the queue
* @param ppInstCtx	-	pointer to instruction context pointer
*
* @ret #STATUS_INVALID_ARGS if pQueue == NULL or ppInstCtx == NULL, #STATUS_QUEUE_EMPTY if queue is empty,
*	   #STATUS_INVALID_QUEUE if unknown error occurred, otherwise #STATUS_SUCCESS
*/
STATUS	Queue_Peek		(PQUEUE pQueue, PPInstCtx ppInstCtx);

/**
* This function returns a pointer to the instruction in the head node of the queue pQueue
* using #Queue_Peek, but also removes the head node from the queue, freeing the node itself
* (not the instruction context).
*
* @param pQueue		-	pointer to the queue
* @param ppInstCtx	-	pointer to instruction context pointer
*
* @ret #STATUS_INVALID_ARGS if pQueue == NULL or ppInstCtx == NULL, #STATUS_QUEUE_EMPTY if queue is empty,
*	   #STATUS_INVALID_QUEUE if unknown error occurred, otherwise #STATUS_SUCCESS
*/
STATUS	Queue_Dequeue	(PQUEUE pQueue, PPInstCtx ppInstCtx);


/**
* This function checks if the queue is full
*
* @param pQueue		-	pointer to the queue
* @param pbIsFull	-	pointer to boolean isFull
*
* @ret #STATUS_INVALID_ARGS if pQueue == NULL or pbIsFull == NULL, otherwise #STATUS_SUCCESS
*/
STATUS	Queue_IsFull	(PQUEUE pQueue, PBOOL pbIsFull);


#endif //QUEUE_H_