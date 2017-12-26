#include <stdio.h>
#include <stdlib.h>
#include "FileParser.h"
#include "Queue.h"
#include "sim.h"
#include "Instructions.h"

/************************************************************************/
/*                  Tomsulo Global Context								*/
/************************************************************************/ 
float 	F[NUM_REGS];
UINT32 	mem[MEMORY_SIZE];
UINT32	PC;
UINT32	CC;
PQUEUE 	pInstQ;
PCHAR	gOpcodeNames[TYPE_CNT] = { "LD","ST","ADD","SUB","MULT","DIV","HALT" };

/************************************************************************/
/*                  MACROS												*/
/************************************************************************/


/**
 * This macro runs a function with its argument, and then checks if the 
 * returned status is SUCCESS (status != 0).
 * Requires an initialized STATUS status variable
 */
#define	runCheckStatusBreak(func,...)										\
{																			\
	status = func(__VA_ARGS__);												\
	if (status)																\
	{																		\
		printf("Function [%s] returned with status %d\n", #func, status);	\
		break;																\
	}																		\
}



/************************************************************************/
/*                  MAIN       									        */
/************************************************************************/

int main(int argc, char** argv)
{
	STATUS		status = STATUS_SUCCESS;
	int			index;
	CONFIG		config = { 0 };
	PInstCtx	currentInst = NULL;
	PCHAR		opcode = NULL;

	do
	{
		printf("\n--- Tomsulo Algorithm Simulator ---\n\n");

		runCheckStatusBreak(FileParser_MeminParser, mem);
		printf("Parsed memin file successfully\n");

		runCheckStatusBreak(FileParser_ConfigParser, &config);
		printf("Parsed config file successfully\n");

		for (index = 0; index < NUM_REGS; index++)
		{
			F[index] = (float)index;
		}
		PC = 0;

		runCheckStatusBreak(Queue_Create, &pInstQ, INSTRUCTION_QUEUE_LEN);

		while (TRUE)
		{
			printf("\n** CC = %d **\n", CC);

			status = Instructions_FetchTwoInstructions(pInstQ, mem, &PC);
			if (status != STATUS_SUCCESS && status != STATUS_QUEUE_FULL)
			{
				printf("[Instructions_FetchTwoInstructions] returned with status %d [PC = %d]\n", status, PC);
				break;
			}

			runCheckStatusBreak(Queue_Dequeue, pInstQ, &currentInst);

			printf("Dequeued instruction type %d (PC = %d)\n", currentInst->opcode,currentInst->pc);


			if (PC == 20)
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