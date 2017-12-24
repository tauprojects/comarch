#include <stdio.h>
#include <stdlib.h>
#include "FileParser.h"
#include "Queue.h"
#include "sim.h"

#define	runCheckStatusBreak(func,...)									\
	status = func(__VA_ARGS__);											\
	if(status)															\
	{																	\
		printf("Function [%s] returned with status %d\n",#func,status);	\
		break;															\
	}																	\



#define ExtractBitsAndCheck(msb,lsb,low,high,dst)						\
	tmp = ExtractBits(inst, msb, lsb);									\
	if (tmp >= low && tmp <= high)										\
	{																	\
		dst = tmp;														\
	}																	\
	else																\
	{																	\
		status = STATUS_INVALID_INSTRUCTION;							\
		break;															\
	}


/** Tomsulo Global Context **/
float 	F[NUM_REGS];
UINT32 	mem[MEMORY_SIZE];
UINT32	PC;
UINT32	CC;
PQUEUE 	pInstQ;



typedef struct _InstCtx 
{
	UINT32	inst;
	UINT32	opcode;
	UINT32	DST;
	UINT32	SRC0;
	UINT32	SRC1;
	UINT32	IMM;
} InstCtx, *PInstCtx;

STATUS ParseAndValidateInstruction(UINT32 inst, PInstCtx pInstCtx)
{
	STATUS	status = STATUS_SUCCESS;
	UINT32	tmp = 0;
	do 
	{
		if (!pInstCtx)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}

		ExtractBitsAndCheck(27, 24, 0, 6, pInstCtx->opcode);

		ExtractBitsAndCheck(23, 20, 0, 15, pInstCtx->DST);

		ExtractBitsAndCheck(19, 16, 0, 15, pInstCtx->SRC0);

		ExtractBitsAndCheck(15, 12, 0, 15, pInstCtx->SRC1);

		pInstCtx->IMM = ExtractBits(inst, 11, 0);

	} while (FALSE);

	return status;
}

STATUS FetchTwoInstructions(VOID)
{
	STATUS status = STATUS_SUCCESS;

	InstCtx cur = { 0 };

	do 
	{
		status = Queue_Enqueue(pInstQ, PC, mem[PC]);
		if (status)
			break;
		PC++;

		status = Queue_Enqueue(pInstQ, PC, mem[PC]);
		if (status)
			break;
		PC++;

	} while (FALSE);

	return status;
}

int main(int argc, char** argv)
{
	STATUS status = STATUS_SUCCESS;
	int index;
	CONFIG config = { 0 };
	
	do
	{
		printf("\n");
		runCheckStatusBreak(FileParser_MeminParser, mem);

		runCheckStatusBreak(FileParser_ConfigParser,&config);
	
		/****/
		for (index = 0; index < NUM_REGS; index++)
		{
			F[index] = (float) index;
		}
		PC = 0;
		/*****/
	
	
		status = Queue_Create(&pInstQ,INSTRUCTION_QUEUE_LEN);

		while (TRUE)
		{
			status = FetchTwoInstructions();
			if (status != STATUS_SUCCESS && status != STATUS_QUEUE_FULL)
			{
				printf("[FetchTwoInstructions] returned with status %d [PC = %d]\n",status,PC);
				break;
			}

			
		}
		


		status = Queue_Destroy(pInstQ);

	} while(FALSE);

	
	
	return 0;
}