
#include "Queue.h"
#include "Instructions.h"
#include <stdlib.h>
#include <string.h>

/************************************************************************/
/*                  MACROS												*/
/************************************************************************/


/**
 * This macro extracts multiple bits from an integer
 * if msb == 31 and lsb == 0, the original integer is returned.
 */
#define ExtractBits(x,msb,lsb)	(msb == 31 && lsb == 0) ? x : ((x >> lsb) & ((1 << (msb - lsb + 1)) - 1));


STATUS Instructions_ParseAndValidateCurrentPC(PInstCtx pInstCtx,UINT32 PC)
{
	STATUS	status = STATUS_SUCCESS;
	UINT32	tmp = 0;
	UINT32	inst = pInstCtx->inst;

	do
	{
		if (!pInstCtx)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}

		memset(pInstCtx, 0, sizeof(InstCtx));

		pInstCtx->opcode = ExtractBits(inst, 27, 24);

		pInstCtx->DST = ExtractBits(inst, 23, 20);

		pInstCtx->SRC0 = ExtractBits(inst, 19, 16);

		pInstCtx->SRC1 = ExtractBits(inst, 15, 12);

		pInstCtx->IMM = ExtractBits(inst, 11, 0);

		pInstCtx->pc = PC;

	} while (FALSE);

	return status;
}

STATUS Instructions_FetchTwoInstructions(PQUEUE pInstQ, PBOOL pWasHaltFetched, PUINT32 mem)
{
	STATUS		status = STATUS_SUCCESS;
	PInstCtx	pCurInst;
	BOOL		isFull = FALSE;
	
	for (int i = 0; i < 2; i++)
	{

		if (!pInstQ || !pWasHaltFetched)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}

		Queue_IsFull(pInstQ, &isFull);

		//if queue is already full, don't fetch anything
		if (isFull)
			break;

		//allocate the instruction context dynamically
		pCurInst = safeMalloc(sizeof(InstCtx));
		if (!pCurInst)
		{
			status = STATUS_MEMORY_FAIL;
			break;
		}

		//actual read from memory
		pCurInst->inst = mem[PC];
		
		//parse intruction
		status = Instructions_ParseAndValidateCurrentPC(pCurInst, PC);
		if (status)
		{
			safeFree(pCurInst);
			break;
		}

		//try to enqueue the instruction, if there's some kind of error, free the instruction context
		status = Queue_Enqueue(pInstQ, pCurInst);
		if (status)
		{
			safeFree(pCurInst);
			break;
		}

		dprintf("Fetched PC %d\n", PC);

		if (pCurInst->opcode == HALT)
		{
			*pWasHaltFetched = TRUE;
			break;
		}

		//Increment the PC only if instruction was fetched and it wasn't HALT
		PC++;
	}

	return status;
}