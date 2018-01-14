#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

#include "mainDefs.h"

extern UINT32	PC;

/**
* This function parses an instruction into an instruction context.
*
* @param	pInstCtx	-	pointer to the instruction context
* @param	PC			-	PC of the parsed instruction
*
* @pre		pInstCtx->inst contains the instruction to be parsed
*
* @ret		#STATUS_INVALID_ARGS if pInstCtx is NULL, otherwise #STATUS_SUCCESS.
*/
STATUS Instructions_ParseAndValidateCurrentPC(PInstCtx pInstCtx, UINT32 PC);




/**
* This function fetches up to two instructions from the memory, using the global PC,
* and enqueues the instruction in the same order to the instruction queue.
* If halts is fetched, no other instruction is fetched afterwards.
*
* @param	pInstQ			-	pointer to the instruction queue
* @param	pWasHaltFetched	-	pointer to a boolean indicating whether halt was fetched.
*
* @ret		#STATUS_INVALID_ARGS if pInstQ == NULL or pWasHaltFetched == NULL, otherwise #STATUS_SUCCESS.
*/
STATUS Instructions_FetchTwoInstructions(PQUEUE pInstQ, PBOOL pWasHalt, PUINT32 mem);

#endif  //INSTRUCTIONS_H_