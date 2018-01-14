
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

extern			globalMemoryCounter;			//DELETE

Register 		F[NUM_REGS];					//Register array
UINT32 			mem[MEMORY_SIZE];				//Memory
UINT32			PC = 0;							//Program Counter, initialized at 0
UINT32			CC = 0;							//Clock cycle Counter, initialized at 0

PQUEUE 			pInstQ;							//Instruction Queue

/* Reservation stations */
pRsvStation		AddRsvStations;					//ADD Reservation stations
pRsvStation		MulRsvStations;					//MUL Reservation stations		
pRsvStation		DivRsvStations;					//DIV Reservation stations

pRsvStation		LoadBuffers;					//Memory Load buffers
pRsvStation		StoreBuffers;					//Memory Store buffers

pRsvStation		reservationStations[FUNC_CNT];	//Array to hold reservation station for polymorphism

_CDB			CDBs[NUM_CDBS];					//CDBs Array


/**
 * This function checks all the CDBs for values matching the same tag in a register.
 */

static VOID CPU_WriteResultToRegister(PBOOL pIsCPUreadyForHalt)
{
	UINT32 index, k;

	//For all registers
	for (index = 0; index < NUM_REGS; index++)
	{
		//If register has tag
		if (F[index].hasTag == TRUE)
		{		
			dprintf("\nRegister F[%d] has tag %s\n", index, F[index].tag->name);

			//halt only the CC after if there is halt
			*pIsCPUreadyForHalt = FALSE;

			//Check all CDBs
			for (k = 0; k < NUM_CDBS; k++)
			{
				//If CDB is currently occupied
				if (CDBs[k].tag != NULL)
				{
					//tag on CDB equals tag of register
					if (CDBs[k].tag == F[index].tag && 
						CDBs[k].inst->pc == F[index].inst->pc)
					{
						//write to register if not store operation
						if (CDBs[k].inst->opcode != ST)
						{
							dprintf("\nRegister F[%d] taking tag %s (value = %f) from CDB %d\n", index, F[index].tag->name, CDBs[k].value, k);
							F[index].value = CDBs[k].value;
						}

						//Clear tag from register, CDB clean is outside
						F[index].hasTag = FALSE;

						break; //continue outer loop to check all other registers
					}
				}
			}
		}
	}


}

/************************************************************************/
/*                  MAIN       									        */
/************************************************************************/

int main(int argc, char** argv)
{

	STATUS			status = STATUS_SUCCESS;
	CONFIG			config = { 0 };
	BOOL			wasHaltIssued = FALSE;
	BOOL			wasHaltFetched = FALSE;
	BOOL			isCPUreadyForHalt = TRUE;
	UINT32			index;

	do
	{
		dprintf("\n--- Tomsulo Algorithm Simulator ---\n\n");

		if (argc < 7) 
		{
			dprintf("Wrong command line arguments.\nRun with: sim cfg.txt memin.txt memout.txt regout.txt traceinst.txt tracecdb.txt\n");
			break;
		}

		//Parse config file
		runCheckStatusBreak(FilesManager_ConfigParser, &config,argv[1]);

		//Parse MEMIN file
		runCheckStatusBreak(FilesManager_MeminParser, mem, argv[2]);
		
		//Initialize / create output files
		runCheckStatusBreak(FilesManager_InitializeOutputFiles, argv[3], argv[4], argv[5], argv[6]);

		//Initialize Reservation stations and memory buffers based on config
		RsvSta_InitializeReservationsStations(&config);

		//Initialize pipeline memory unit based on config
		CPU_InitializeMemoryUnit(&config);

		//Initialize registers
		for (index = 0; index < NUM_REGS; index++)
		{
			F[index].value = (FLOAT)index;
			F[index].hasTag = FALSE;
			F[index].tag = NULL;
		}

		//Initialize instruction queue
		runCheckStatusBreak(Queue_Create, &pInstQ, INSTRUCTION_QUEUE_LEN);

		while (TRUE)
		{
			dprintf("\n************************************** CC = %d ************************************\n", CC);

			//Assume we are ready for halt, if not, then somewhere it will be falsed
			isCPUreadyForHalt = TRUE;

			//no reason to fetch more instructions after halt was fetched, no branch
			if (wasHaltFetched == FALSE)
			{

				//Fetch up to two instructions and add them to the instructions queue if there's room
				Instructions_FetchTwoInstructions(pInstQ, &wasHaltFetched, mem);

				//if the first CC, cannot do anything yet because fetch takes full CC
				if (CC == 0)
				{
					CC++;
					continue;
				}
			}

			//if halt was issued already don't issue more instructions, wait for the current ones to end processing
			if (wasHaltIssued == FALSE)
			{ 
				//Issue instructions to reservation stations
				RsvSta_IssueInstToRsvStations(&config, &wasHaltIssued, pInstQ, CC);
			}

			//Process Functional Units
			CPU_ProcessFunctionalUnits(&isCPUreadyForHalt);

			//Process memory unit
			CPU_ProcessMemoryUnit(&isCPUreadyForHalt);

			//Check if there's values ready on the CDB for any reservation station
			RsvSta_CheckIfRsvStationCanGetDataFromCDB(&isCPUreadyForHalt,&config);

			//Write from CDB to registers
			CPU_WriteResultToRegister(&isCPUreadyForHalt);
			
			//Write TraceCDB file every CC
			FilesManager_WriteTracedb(CDBs, CC);

			//Clear CDBs every CC because all needed was passed either to reservation station or register
			memset(CDBs, 0, sizeof(CDBs));

			//If halt was issued and the CPU is ready for halt, break the loop
			if (wasHaltIssued == TRUE && isCPUreadyForHalt == TRUE)
			{
				dprintf("\nHALT detected, BREAKING!!\n");
				break;
			}

			CC++;
		}

		/************************************************************************/
		/* DEBUG printing */
		dprintf("\nProgram finished CC = %d || PC = %d\n", CC, PC);
		dprintf("\n\nFinal Register Values:\n^^^^^^^^^^^^^^^^^^^^^^\n\n");
		for (index = 0; index < NUM_REGS; index++)
			dprintf("F[%d] = %f\n", index, F[index].value);
		/************************************************************************/


	} while (FALSE);


	//Write REGOUT file
	FilesManager_WriteRegisters(F);

	//Write MEMOUT file
	FilesManager_WriteMemout(mem);

	//Write TRACEINST file
	FilesManager_WriteTraceinst();

	//Finalize instruction queue
	Queue_Destroy(pInstQ);

	//Free all dynamically allocated memory pointers
	cleanMemory();

	dprintf("\n*** Memory Counter = %d\n", globalMemoryCounter);

	return 0;
}