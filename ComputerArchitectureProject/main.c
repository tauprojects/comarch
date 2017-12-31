#include <stdio.h>
#include <stdlib.h>
#include "FilesManager.h"
#include "Queue.h"
#include "Instructions.h"

/************************************************************************/
/*                  Tomsulo Global Context								*/
/************************************************************************/

extern globalMemoryCounter;

Register 		F[NUM_REGS];
UINT32 			mem[MEMORY_SIZE];
UINT32			PC = 0;
UINT32			CC = 0;
PQUEUE 			pInstQ;
PCHAR			gOpcodeNames[TYPE_CNT] = { "LD","ST","ADD","SUB","MULT","DIV","HALT" };

/* Reservation stations */
pRsvStation		reservationStations[FUNC_CNT];

pRsvStation		AddRsvStations;
pRsvStation		MulRsvStations;
pRsvStation		DivRsvStations;

pRsvStation		LoadBuffers;
pRsvStation		StoreBuffers;

pFunctionUnit	memoryUnit;

_CDB			CDBs[4];
/************************************************************************/
/*                  MACROS												*/
/************************************************************************/

#define funame(j)			j == 0 ? "ADD" : (j==1 ? "MULT" : (j==2 ? "DIV" : ""))
#define opcode(j)			gOpcodeNames[j];

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

VOID InitializeOneRsvStation(pRsvStation* pRsvStationsTypeArray, UINT32 nr_reservation,
	CPCHAR name, UINT32 nr_units, eInstType type, UINT32 delay)
{
	UINT32		i;
	pRsvStation	RsvStationsTypeArray;

	RsvStationsTypeArray = safeCalloc(nr_reservation, sizeof(RsvStation));
	*pRsvStationsTypeArray = RsvStationsTypeArray;
	for (i = 0; i < nr_reservation; i++)
	{
		(RsvStationsTypeArray)[i].type = type;
		RsvStationsTypeArray[i].numOfRsvStationsFromThisType = nr_reservation;
		snprintf(RsvStationsTypeArray[i].name, 10, "%s%d", name, i);
	}
	reservationStations[type] = RsvStationsTypeArray;

	if (type != LD && type != ST)
	{
		RsvStationsTypeArray->functionalUnits = safeCalloc(nr_units, sizeof(FunctionUnit));
		for (i = 0; i < nr_units; i++)
		{
			RsvStationsTypeArray->functionalUnits[i].delay = delay;
			RsvStationsTypeArray->functionalUnits[i].numOfFunctionalUnitsFromThisType = nr_units;
			RsvStationsTypeArray->functionalUnits[i].type = type;
		}
	}
}

VOID InitializeReservationsStations(PCONFIG pConfig)
{
	InitializeOneRsvStation(&AddRsvStations, pConfig->add_nr_reservation, "ADD",
		pConfig->add_nr_units, ADD, pConfig->add_delay);

	reservationStations[SUB] = AddRsvStations;	//so SUB instructions would go to ADD rsv stations also

	InitializeOneRsvStation(&MulRsvStations, pConfig->mul_nr_reservation, "MUL",
		pConfig->mul_nr_units, MULT, pConfig->mul_delay);

	InitializeOneRsvStation(&DivRsvStations, pConfig->div_nr_reservation, "DIV",
		pConfig->div_nr_units, DIV, pConfig->div_delay);

	InitializeOneRsvStation(&LoadBuffers, pConfig->mem_nr_load_buffers, "LD",
		0, LD, 0);

	InitializeOneRsvStation(&StoreBuffers, pConfig->mem_nr_store_buffers, "ST",
		0, ST, 0);

	//only one memory unit
	memoryUnit = safeCalloc(1, sizeof(FunctionUnit));
	memoryUnit->delay = pConfig->mem_delay;
	memoryUnit->numOfFunctionalUnitsFromThisType = 1;

}

UINT32	OpcodeToNumberOfReservationStations(eInstType opcode, PCONFIG config)
{
	UINT32 ret;

	switch (opcode)
	{
	case ADD:
	case SUB:
		ret = config->add_nr_reservation;
		break;

	case MULT:
		ret = config->mul_nr_reservation;
		break;

	case DIV:
		ret = config->div_nr_reservation;
		break;

	case LD:
		ret = config->mem_nr_load_buffers;
		break;

	case ST:
		ret = config->mem_nr_store_buffers;
		break;
	}

	return ret;
}

VOID CPU_IssueInstToRsvStations(PCONFIG pConfig, PBOOL pWasHolt)
{
	UINT32		index, j, numberOfRsvStations;
	BOOL		instructionIssued;
	pRsvStation currentRsvStations;
	PInstCtx	currentInst;

	for (j = 0; j < 2; j++)
	{
		Queue_Peek(pInstQ, &currentInst);

		printf("\n** Working on instruction <PC=%d,OPCODE=%d>\n",
			currentInst->pc, currentInst->opcode);
		if (currentInst->opcode != HALT)
		{
			currentRsvStations = reservationStations[currentInst->opcode];
			numberOfRsvStations = OpcodeToNumberOfReservationStations(currentInst->opcode, pConfig);
			instructionIssued = FALSE;

			for (index = 0; index < numberOfRsvStations; index++)
			{
				if (currentRsvStations[index].busy == FALSE)
				{
					//Found empty reservation station
					//at this point can dequeue from instruction queue
					Queue_Dequeue(pInstQ, &currentInst);

					currentInst->cycleIssued = CC;

					currentInst->tag = &currentRsvStations[index];
					// insert instruction to reservation station
					currentRsvStations[index].pInstruction = currentInst;
					currentRsvStations[index].busy = TRUE;
					instructionIssued = TRUE;

					FilesManager_AddToIssueArray(currentInst);

					printf("\n** Added instruction <PC=%d,OPCODE=%d> to station %s\n",
						currentInst->pc, currentInst->opcode, currentRsvStations[index].name);

					//Update destination tag in register
					//doesn't matter if has tag already

					if (currentInst->opcode != LD)
					{
						if (F[currentInst->SRC1].hasTag == FALSE)
						{
							currentRsvStations[index].Vk = F[currentInst->SRC1].value;
						}
						else
						{
							currentRsvStations[index].Qk = F[currentInst->SRC1].tag;
							printf("\n RsvStation %s Qk took tag from F[%d] = %s\n", currentRsvStations[index].name, currentInst->SRC1, F[currentInst->SRC1].tag->name);
						}

						if (currentInst->opcode != ST)
						{
							if (F[currentInst->SRC0].hasTag == FALSE)
							{
								currentRsvStations[index].Vj = F[currentInst->SRC0].value;
							}
							else
							{
								currentRsvStations[index].Qj = F[currentInst->SRC0].tag;
								printf("\n RsvStation %s Qj took tag from F[%d] = %s\n", currentRsvStations[index].name, currentInst->SRC0, F[currentInst->SRC0].tag->name);
							}
						}
					}

					F[currentInst->DST].hasTag = TRUE;
					F[currentInst->DST].tag = &currentRsvStations[index];
					F[currentInst->DST].inst = currentInst;
					printf("\nWrote tag %s to register F[%d] \n", F[currentInst->DST].tag->name, currentInst->DST);


					break;
				}
				else
				{
					printf("\n** Station %s is BUSY\n", currentRsvStations[index].name);
				}

			}

			if (instructionIssued == FALSE)
			{
				//Don't do another loop of check instruction 
				//because it will be on the same instruction
				break;
			}
		}

		else
		{
			//// HALTTTT
			printf("Instruction (PC = %d) is HALT\n", currentInst->pc);
			*pWasHolt = TRUE;
			break;
		}
	}
}

VOID CPU_ProcessFunctionalUnits(PBOOL pisCPUreadyForHalt)
{
	UINT32			index, j, k;
	pRsvStation		relevantReservationStations;
	pFunctionUnit	FU_array;
	pFunctionUnit	pCurrentFU;
	pFunctionUnit	FUs[3] = { AddRsvStations->functionalUnits,
								MulRsvStations->functionalUnits,
								DivRsvStations->functionalUnits };


	//pass on every type of functional unit
	for (j = 0; j < 3; j++)
	{

		FU_array = FUs[j];
		//pass on every functional unit from the specific type
		for (index = 0; index < FU_array->numOfFunctionalUnitsFromThisType; index++)
		{

			pCurrentFU = &FU_array[index];
			if (pCurrentFU->busy == TRUE)
			{
				*pisCPUreadyForHalt = FALSE;

				if (pCurrentFU->clockCycleCounter == pCurrentFU->delay - 1)
				{
					printf("\nFuncUnit %s.%d finished\n", funame(j), index);

					if (pCurrentFU->pInstruction->cycleExecutionEnd == 0)
					{
						pCurrentFU->pInstruction->cycleExecutionEnd = CC;
					}
					else
					{ //it didn't end in this CC

						//same index as the type of the functional unit,
						//try to write to CDB
						if (CDBs[j].tag == NULL)
						{
							//if relevant CDB is empty
							printf("FuncUnit %s.%d wrote value %f to CDB %d\n", funame(j), index, pCurrentFU->DST, j);

							//take tag from the instruction
							CDBs[j].tag = pCurrentFU->pInstruction->tag;
							CDBs[j].inst = pCurrentFU->pInstruction;
							CDBs[j].value = pCurrentFU->DST;
							CDBs[j].CCupdated = CC;
							pCurrentFU->pInstruction->cycleWriteCDB = CC;

							//FilesManager_WriteTraceinst(pCurrentFU->pInstruction);

							//safeFree(pCurrentFU->pInstruction);
							//finished with instruction :)

							pCurrentFU->clockCycleCounter = 0;
							pCurrentFU->busy = FALSE;
					}
					
					}
					//if CDBs[j].tag is not NULL then just wait, don't clock cycle counter
				}
				else
				{
					pCurrentFU->clockCycleCounter++;
					printf("\nFuncUnit %s.%d counter = %d\n", funame(j), index, pCurrentFU->clockCycleCounter);
				}
			}
			else //specific FU is empty
			{
				relevantReservationStations = reservationStations[pCurrentFU->type];

				//check reservations stations from this type and take first valid instruction
				for (k = 0; k < relevantReservationStations->numOfRsvStationsFromThisType; k++)
				{
					if (relevantReservationStations[k].busy == TRUE)
					{

						//don't move instruction from reservation station to functional unit in the same CC
						if (relevantReservationStations[k].pInstruction->cycleIssued == CC)
							continue;

						*pisCPUreadyForHalt = FALSE;
						if (relevantReservationStations[k].Qj == NULL &&
							relevantReservationStations[k].Qk == NULL)
						{
							printf("\nFuncUnit %s.%d taking PC %d from RsvStation %s\n", funame(j), index, relevantReservationStations[k].pInstruction->pc, relevantReservationStations[k].name);

							//if they are both empty, namely, if values are relevant in reservation station
							pCurrentFU->busy = TRUE;

							pCurrentFU->pInstruction = relevantReservationStations[k].pInstruction;

							pCurrentFU->pInstruction->cycleExecutionStart = CC;

							relevantReservationStations[k].pInstruction = NULL;

							pCurrentFU->SRC0 = relevantReservationStations[k].Vj;
							pCurrentFU->SRC1 = relevantReservationStations[k].Vk;

							pCurrentFU->clockCycleCounter++; //this cycle counts

							relevantReservationStations[k].busy = FALSE;

							// Do the actual calculation and save the result inside the functional unit
							switch (pCurrentFU->pInstruction->opcode)
							{
							case ADD:

								pCurrentFU->DST = pCurrentFU->SRC0 + pCurrentFU->SRC1;
								printf("\nADD happened - DST = %f\n", pCurrentFU->DST);

								break;
							case SUB:

								pCurrentFU->DST = pCurrentFU->SRC0 - pCurrentFU->SRC1;
								printf("\nSUB happened - DST = %f\n", pCurrentFU->DST);

								break;
							case MULT:

								pCurrentFU->DST = pCurrentFU->SRC0 * pCurrentFU->SRC1;
								printf("\nMULT happened - DST = %f\n", pCurrentFU->DST);

								break;
							case DIV:

								pCurrentFU->DST = pCurrentFU->SRC0 / pCurrentFU->SRC1;
								printf("\nDIV happened - DST = %f\n", pCurrentFU->DST);

								break;
							default:
								printf("\nWRONG INSTRUCTION\n");
								break;
							}
							////////

							break;
						}
					}
				}
			}
		}
	}
}

/************************************************************************/
/*                  MAIN       									        */
/************************************************************************/

VOID CPU_CheckIfRsvStationCanGetDataFromCDB(PBOOL pIsCPUreadyForHalt, PCONFIG pConfig)
{
	UINT32		j, index, k, numberOfRsvStations;
	pRsvStation currentRsvStations;
	pCDB		CDB;

	//pass on all the reservation station types
	for (j = 0; j < FUNC_CNT; j++)
	{
		if (j == LD || j == SUB) //LD is not relevant, SUB is the same as load
			continue;

		currentRsvStations = reservationStations[j];

		numberOfRsvStations = OpcodeToNumberOfReservationStations(j, pConfig);

		//all the reservation stations from this type
		for (index = 0; index < numberOfRsvStations; index++)
		{
			if (currentRsvStations[index].busy == TRUE)
			{
				printf("\nRsvStation %s has instruction PC=%d\n", currentRsvStations[index].name, currentRsvStations[index].pInstruction->pc);

				*pIsCPUreadyForHalt = FALSE;

				for (k = 0; k < 4; k++)	//number of CDBs
				{
					//check the relevant CDB if there's relevant information

					CDB = &CDBs[k];

					if (CDB->tag != NULL)
					{
						if (currentRsvStations[index].Qj == CDB->tag)
						{
							printf("\nRsvStation %s took value Vj from CDB %d for PC=%d\n", currentRsvStations[index].name, k, currentRsvStations[index].pInstruction->pc);
							currentRsvStations[index].Qj = NULL;
							currentRsvStations[index].Vj = CDB->value;
						}
						else if (currentRsvStations[index].Qk == CDB->tag)
						{
							printf("\nRsvStation %s took value Vk from CDB %d for PC=%d\n", currentRsvStations[index].name, k, currentRsvStations[index].pInstruction->pc);
							currentRsvStations[index].Qk = NULL;
							currentRsvStations[index].Vk = CDB->value;
						}
					}
				}

			}
		}
	}
}

VOID CPU_ProcessMemoryUnit(PBOOL pIsCPUreadyForHalt)
{
	UINT32		j, k;
	pRsvStation currentBuffer;
	pRsvStation buffers[2] = { LoadBuffers, StoreBuffers };

	if (memoryUnit->busy == TRUE)
	{
		printf("\nmemoryUnit is BUSY with instruction PC = %d\n", memoryUnit->pInstruction->pc);
		*pIsCPUreadyForHalt = FALSE;
		if (memoryUnit->clockCycleCounter == memoryUnit->delay - 1)
		{
			printf("\nmemoryUnit is finished instruction PC = %d\n", memoryUnit->pInstruction->pc);

			memoryUnit->pInstruction->cycleExecutionEnd = CC;
			//same index as the type of the functional unit,
			//try to write to CDB
			if (CDBs[3].tag == NULL)
			{
				//if relevant CDB is empty
				printf("\nmemoryUnit wrote PC = %d to CDB 3\n", memoryUnit->pInstruction->pc);

				//take tag from the instruction
				CDBs[3].tag = memoryUnit->pInstruction->tag;
				CDBs[3].value = memoryUnit->DST;

				memoryUnit->pInstruction->cycleWriteCDB = CC;

				//FilesManager_WriteTraceinst(memoryUnit->pInstruction);

				//safeFree(memoryUnit->pInstruction);
				//finished with instruction :)

				memoryUnit->clockCycleCounter = 0;
				memoryUnit->busy = FALSE;
			}
			//if CDBs[3].tag is not NULL then just wait, don't clock cycle counter
		}
		else
		{
			memoryUnit->clockCycleCounter++;
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
							mem[memoryUnit->pInstruction->IMM] = Float2Hex(currentBuffer[k].Vk);
							break;

						default:
							printf("WRONG INSTRUCTION\n");
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
			printf("\nRegister F[%d] has tag %s\n", index, F[index].tag->name);
			*pIsCPUreadyForHalt = FALSE;
			for (k = 0; k < 4; k++) //pass on all CDB's
			{
				if (CDBs[k].tag != NULL)
				{
					//tag on CDB equals tag of register
					if (CDBs[k].tag == F[index].tag && 
						CDBs[k].inst->pc == F[index].inst->pc)
					{
						//write to register 
						printf("\nRegister F[%d] taking tag %s (value = %f) from CDB %d\n", index, F[index].tag->name, CDBs[k].value, k);

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
			printf("Wrong command line arguments.\nRun with: sim cfg.txt memin.txt memout.txt regout.txt traceinst.txt tracecdb.txt\n");
			break;
		}

		runCheckStatusBreak(FilesManager_ConfigParser, &config,argv[1]);

		runCheckStatusBreak(FilesManager_MeminParser, mem, argv[2]);

		runCheckStatusBreak(FilesManager_InitializeOutputFiles, argv[3], argv[4], argv[5], argv[6]);

		InitializeReservationsStations(&config);

		for (index = 0; index < NUM_REGS; index++)
		{
			F[index].value = (float)index;
			F[index].hasTag = FALSE;
			F[index].tag = NULL;
		}

		runCheckStatusBreak(Queue_Create, &pInstQ, INSTRUCTION_QUEUE_LEN);

		while (TRUE)
		{
			printf("\n************************************** CC = %d ************************************\n", CC);
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
				CPU_IssueInstToRsvStations(&config, &wasHalt);
			}

			/* Process Functional Units */
			CPU_ProcessFunctionalUnits(&isCPUreadyForHalt);

			/* Check if there's values ready on the CDB for any reservation station */
			CPU_CheckIfRsvStationCanGetDataFromCDB(&isCPUreadyForHalt,&config);

			/* Process memory unit */
			CPU_ProcessMemoryUnit(&isCPUreadyForHalt);

			/* MEMORY UNIT SHOULD PE PIPELINED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

			CPU_WriteResultToRegister(&isCPUreadyForHalt);
			
			FilesManager_WriteTracedb(CDBs, CC);

			//Clear CDBs every CC because all needed was passed either to reservation station or register
			memset(CDBs, 0, sizeof(CDBs));

			if (wasHalt == TRUE && isCPUreadyForHalt == TRUE)
			{
				printf("\nHALT detected, BREAKING!!\n");
				break;
			}

			CC++;
		}

		printf("\n\nFinal Register Values:\n^^^^^^^^^^^^^^^^^^^^^^\n\n");
		for (index = 0; index < NUM_REGS; index++)
			printf("F[%d] = %.1f\n", index, F[index].value);

	} while (FALSE);

	FilesManager_WriteRegisters(F);

	FilesManager_WriteMemout(mem);
	FilesManager_WriteTraceinst();

	Queue_Destroy(pInstQ);

	cleanMemory();

	printf("\n*** Memory Counter = %d\n", globalMemoryCounter);
	//printf("\npress any key to exit...\n");
	//getchar();

	return 0;
}