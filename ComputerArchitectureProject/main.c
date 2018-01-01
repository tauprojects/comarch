
#include <stdio.h>
#include <stdlib.h>
#include "FilesManager.h"
#include "Queue.h"
#include "Instructions.h"
#include "ReservationStations.h"
#include "FunctionalUnits.h"

/************************************************************************/
/*                  Tomsulo Global Context								*/
/************************************************************************/

extern			globalMemoryCounter;

Register 		F[NUM_REGS];
UINT32 			mem[MEMORY_SIZE];
UINT32			PC = 0;
UINT32			CC = 0;
PQUEUE 			pInstQ;

/* Reservation stations */
pRsvStation		reservationStations[FUNC_CNT];

pRsvStation		AddRsvStations;
pRsvStation		MulRsvStations;
pRsvStation		DivRsvStations;

pRsvStation		LoadBuffers;
pRsvStation		StoreBuffers;

pFunctionUnit	memoryUnit;

_CDB			CDBs[NUM_CDBS];


/************************************************************************/
/*                  MAIN       									        */
/************************************************************************/


VOID CPU_ProcessMemoryUnit(PBOOL pIsCPUreadyForHalt)
{
	UINT32		j, k;
	pRsvStation currentBuffer;
	pRsvStation buffers[2] = { LoadBuffers, StoreBuffers };

	if (memoryUnit->busy == TRUE)
	{

		dprintf("\nmemoryUnit is BUSY with instruction PC = %d\n", memoryUnit->pInstruction->pc);
		*pIsCPUreadyForHalt = FALSE;
		if (memoryUnit->clockCycleCounter == memoryUnit->delay - 1)
		{
			dprintf("\nmemoryUnit is finished instruction PC = %d\n", memoryUnit->pInstruction->pc);

			memoryUnit->pInstruction->cycleExecutionEnd = CC;
			//same index as the type of the functional unit,
			//try to write to CDB
			
			if (memoryUnit->pInstruction->opcode == ST)
			{
				//actual write to memory
				mem[memoryUnit->pInstruction->tag->Address] = Float2Hex(memoryUnit->DST);
				dprintf("\nMemoryUnit wrote value %f <%X> to MEM[%d]\n",memoryUnit->DST, mem[memoryUnit->pInstruction->tag->Address], memoryUnit->pInstruction->tag->Address);
			}
			
			if (CDBs[3].tag == NULL)
			{
				//if relevant CDB is empty
				dprintf("\nMemoryUnit wrote value %f to CDB 3\n", memoryUnit->DST);

				//take tag from the instruction
				CDBs[3].tag = memoryUnit->pInstruction->tag;
				CDBs[3].inst = memoryUnit->pInstruction;
				CDBs[3].value = memoryUnit->DST;
				CDBs[3].CCupdated = CC;
				memoryUnit->pInstruction->cycleWriteCDB = CC;

				memoryUnit->pInstruction->tag->pInstruction = NULL; //clean reservation station
				memoryUnit->pInstruction->tag->busy = FALSE;

				memoryUnit->clockCycleCounter = 0;
				memoryUnit->busy = FALSE;
			}
			//if CDBs[3].tag is not NULL then just wait, don't clock cycle counter
		}
		else
		{
			memoryUnit->clockCycleCounter++;
			dprintf("\nMemoryUnit counter = %d\n", memoryUnit->clockCycleCounter);

		}
	}
	else //memory unit is empty
	{
		for (j = 0; j < 2; j++)
		{
			currentBuffer = buffers[j];

			for (k = 0; k < currentBuffer->numOfRsvStationsFromThisType; k++)
			{
				if (currentBuffer[k].busy == TRUE)
				{
					*pIsCPUreadyForHalt = FALSE;
					//relevant only for store, will always be NULL for load
					if (currentBuffer[k].Qk == NULL)
					{
						dprintf("\nMemoryUnit taking PC %d from RsvStation %s\n", currentBuffer[k].pInstruction->pc, currentBuffer[k].pInstruction->tag->name);

						//if the store value is valid
						memoryUnit->busy = TRUE;

						memoryUnit->pInstruction = currentBuffer[k].pInstruction;

						memoryUnit->pInstruction->cycleExecutionStart = CC;
						memoryUnit->clockCycleCounter++; //this cycle counts

						currentBuffer[k].pInstruction = NULL;

						currentBuffer[k].busy = FALSE;

						switch (memoryUnit->pInstruction->opcode)
						{
						case LD:
							memoryUnit->DST = Hex2Float(mem[memoryUnit->pInstruction->IMM]);
							break;

						case ST:
							memoryUnit->DST = currentBuffer[k].Vk;
							break;

						default:
							dprintf("WRONG INSTRUCTION\n");
							break;
						}
						////////

						break;
					}
				}

			}

			if (memoryUnit->busy == TRUE)
				break; //break outer loop if instruction was already treated
		}
	}
}

VOID CPU_WriteResultToRegister(PBOOL pIsCPUreadyForHalt)
{
	UINT32 index, k;

	for (index = 0; index < NUM_REGS; index++)
	{
		if (F[index].hasTag == TRUE)
		{
			
			//breakpoint;
			dprintf("\nRegister F[%d] has tag %s\n", index, F[index].tag->name);
			*pIsCPUreadyForHalt = FALSE;
			for (k = 0; k < NUM_CDBS; k++) //pass on all CDB's
			{
				if (CDBs[k].tag != NULL)
				{
					//tag on CDB equals tag of register
					if (CDBs[k].tag == F[index].tag && 
						CDBs[k].inst->pc == F[index].inst->pc)
					{
						//write to register 
						dprintf("\nRegister F[%d] taking tag %s (value = %f) from CDB %d\n", index, F[index].tag->name, CDBs[k].value, k);

						F[index].value = CDBs[k].value;
						F[index].hasTag = FALSE;

						break; //continue outer loop
					}
				}
			}
		}
	}


}

int main(int argc, char** argv)
{

	STATUS			status = STATUS_SUCCESS;
	CONFIG			config = { 0 };
	BOOL			wasHalt = FALSE;
	BOOL			isCPUreadyForHalt = TRUE;
	UINT32			index;

	do
	{
		printf("\n--- Tomsulo Algorithm Simulator ---\n\n");

		if (argc < 7) 
		{
			dprintf("Wrong command line arguments.\nRun with: sim cfg.txt memin.txt memout.txt regout.txt traceinst.txt tracecdb.txt\n");
			break;
		}

		runCheckStatusBreak(FilesManager_ConfigParser, &config,argv[1]);

		runCheckStatusBreak(FilesManager_MeminParser, mem, argv[2]);

		runCheckStatusBreak(FilesManager_InitializeOutputFiles, argv[3], argv[4], argv[5], argv[6]);

		RsvSta_InitializeReservationsStations(&config);

		for (index = 0; index < NUM_REGS; index++)
		{
			F[index].value = (float)index;
			F[index].hasTag = FALSE;
			F[index].tag = NULL;
		}

		runCheckStatusBreak(Queue_Create, &pInstQ, INSTRUCTION_QUEUE_LEN);

		while (TRUE)
		{
			dprintf("\n************************************** CC = %d ************************************\n", CC);
			isCPUreadyForHalt = TRUE;

			/**
			 * Fetch up to two instructions and add them to the instructions queue
			 * if there's room.
			 */
			if (wasHalt == FALSE)
			{
				Instructions_FetchTwoInstructions(pInstQ, mem, &PC);

				//if the first CC, cannot do anything yet because fetch takes full CC
				if (CC == 0)
				{
					CC++;
					continue;
				}

				/* Issue to reservation stations */
				RsvSta_IssueInstToRsvStations(&config, &wasHalt, pInstQ, CC);
			}

			/* Process Functional Units */
			CPU_ProcessFunctionalUnits(&isCPUreadyForHalt);

			/* Process memory unit */
			CPU_ProcessMemoryUnit(&isCPUreadyForHalt);

			/* Check if there's values ready on the CDB for any reservation station */
			RsvSta_CheckIfRsvStationCanGetDataFromCDB(&isCPUreadyForHalt,&config);

			/* MEMORY UNIT SHOULD PE PIPELINED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

			CPU_WriteResultToRegister(&isCPUreadyForHalt);
			
			FilesManager_WriteTracedb(CDBs, CC);

			//Clear CDBs every CC because all needed was passed either to reservation station or register
			memset(CDBs, 0, sizeof(CDBs));

			if (wasHalt == TRUE && isCPUreadyForHalt == TRUE)
			{
				dprintf("\nHALT detected, BREAKING!!\n");
				break;
			}

			CC++;
		}

		printf("\n\nFinal Register Values:\n^^^^^^^^^^^^^^^^^^^^^^\n\n");
		for (index = 0; index < NUM_REGS; index++)
			printf("F[%d] = %f\n", index, F[index].value);

	} while (FALSE);

	FilesManager_WriteRegisters(F);

	FilesManager_WriteMemout(mem);
	FilesManager_WriteTraceinst();

	Queue_Destroy(pInstQ);

	cleanMemory();

	printf("\n*** Memory Counter = %d\n", globalMemoryCounter);

	return 0;
}