
#include "Queue.h"
#include "Instructions.h"
#include <stdlib.h>





/************************************************************************/
/*                  MACROS												*/
/************************************************************************/


/**
 * This macro extracts multiple bits from an integer
 * if msb == 31 and lsb == 0, the original integer is returned.
 */
#define ExtractBits(x,msb,lsb)	(msb == 31 && lsb == 0) ? x : ((x >> lsb) & ((1 << (msb - lsb + 1)) - 1));


/**
* This macro extract specific bits using the ExtractBits macro from an instruction,
* and checks whether the number is within its legal boundaries.
* If not, breaks and assigns STATUS_INVALID_INSTRUCTION to status.
* Requires an initialized STATUS status variable.
*/
#define ExtractBitsAndCheck(msb,lsb,low,high,dst)							\
{																			\
	tmp = ExtractBits(inst, msb, lsb);										\
	if (tmp >= low && tmp <= high)											\
	{																		\
		dst = tmp;															\
	}																		\
	else																	\
	{																		\
		printf("Instruction section %s is invalid - got %d\n",#dst, tmp);	\
		status = STATUS_INVALID_INSTRUCTION;								\
		break;																\
	}																		\
}

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

		ExtractBitsAndCheck(27, 24, 0, 6, pInstCtx->opcode);

		ExtractBitsAndCheck(23, 20, 0, 15, pInstCtx->DST);

		ExtractBitsAndCheck(19, 16, 0, 15, pInstCtx->SRC0);

		ExtractBitsAndCheck(15, 12, 0, 15, pInstCtx->SRC1);

		pInstCtx->IMM = ExtractBits(inst, 11, 0);

		pInstCtx->pc = PC;

	} while (FALSE);

	return status;
}

//Need to free instructions from queue afterwards
STATUS Instructions_FetchTwoInstructions(PQUEUE pInstQ, PUINT32 mem, PUINT32 pPC)
{
	STATUS		status = STATUS_SUCCESS;
	PInstCtx	pCurInst = { 0 };
	BOOL		isFull = FALSE;
	UINT32		PC = 0;
	


	for (int i = 0; i < 2; i++)
	{

		if (!pInstQ || !pPC)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}

		PC = *pPC;

		Queue_IsFull(pInstQ, &isFull);

		pCurInst = safeMalloc(sizeof(InstCtx));
		if (!pCurInst)
		{
			status = STATUS_MEMORY_FAIL;
			break;
		}

		pCurInst->inst = mem[PC];

		status = Instructions_ParseAndValidateCurrentPC(pCurInst, PC);
		if (status)
		{
			safeFree(pCurInst);
			break;
		}

		status = Queue_Enqueue(pInstQ, pCurInst);
		if (status)
		{
			safeFree(pCurInst);
			break;
		}

		printf("Fetched PC %d\n", PC);

		*pPC = PC + 1;
	}

	return status;
}