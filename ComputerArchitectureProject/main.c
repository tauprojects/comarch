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





STATUS ParseAndValidateCurrentPC(PInstCtx pInstCtx)
{
	STATUS	status = STATUS_SUCCESS;
	UINT32	tmp = 0;
	UINT32	inst = mem[PC];

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

		pInstCtx->pc = PC;

	} while (FALSE);

	return status;
}

//Need to free instructions from queue afterwards
STATUS FetchTwoInstructions(VOID)
{
	STATUS status = STATUS_SUCCESS;
	PInstCtx pCurInst = { 0 };
	BOOL isFull = FALSE;

	for (int i = 0; i < 2; i++)
	{
		if (Queue_IsFull(pInstQ, &isFull))
			break;

		pCurInst = malloc(sizeof(InstCtx));
		if (!pCurInst)
		{
			status = STATUS_MEMORY_FAIL;
			break;
		}

		ParseAndValidateCurrentPC(pCurInst);

		printf("Fetched PC %d\n", PC);
		PC++;
	}

	return status;
}

int main(int argc, char** argv)
{
	STATUS status = STATUS_SUCCESS;
	int index;
	CONFIG config = { 0 };

	do
	{
		printf("\n--- Tomasulu Algorithm Simulator ---\n\n");

		runCheckStatusBreak(FileParser_MeminParser, mem);
		printf("Parsed memin file successfully\n");

		runCheckStatusBreak(FileParser_ConfigParser, &config);
		printf("Parsed config file successfully\n");

		/****/
		for (index = 0; index < NUM_REGS; index++)
		{
			F[index] = (float)index;
		}
		PC = 0;
		/*****/

		runCheckStatusBreak(Queue_Create, &pInstQ, INSTRUCTION_QUEUE_LEN);

		while (TRUE)
		{
			printf("\n** CC = %d **\n", CC);
			status = FetchTwoInstructions();
			if (status != STATUS_SUCCESS && status != STATUS_QUEUE_FULL)
			{
				printf("[FetchTwoInstructions] returned with status %d [PC = %d]\n", status, PC);
				break;
			}

			if (PC == 16)
				break;

			CC++;
			Sleep(100);	//to show progress in cmd
		}



		Queue_Destroy(pInstQ);

	} while (FALSE);

	printf("\npress any key to exit...\n");
	getchar();

	return 0;
}