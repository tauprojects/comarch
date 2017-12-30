#include <stdio.h>
#include <stdlib.h>
#include "FileParser.h"
#include "Queue.h"
#include "sim.h"
#include "Instructions.h"
#include "Instructions.h"
#include "time.h"
/************************************************************************/
/*                  Tomsulo Global Context								*/
/************************************************************************/

extern globalMemoryCounter;

Register 		F[NUM_REGS];
UINT32 			mem[MEMORY_SIZE];
UINT32			PC;
UINT32			CC;
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

#define breakpoint			//printf("## BREAKPOINT: %s | %d\n",__FUNCTION__,__LINE__)
#define funame(j)			j == 0 ? "ADD" : (j==1 ? "MULT" : (j==2 ? "DIV" : ""))
#define opcode(j)			j == 0 ? "LD" : (j==1 ? "ST" : (j==2 ? "ADD" : (j==3 ? "SUB" : (j==4 ? "MULT" : (j==5 ? "DIV" : (j==6 ? "DIV" : ""))))))
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


STATUS InitializeReservationsStations(PCONFIG pConfig)
{
	STATUS		status = STATUS_SUCCESS;
	UINT32		i;
	do
	{
		AddRsvStations = safeCalloc(pConfig->add_nr_reservation, sizeof(RsvStation));
		for (i = 0; i < pConfig->add_nr_reservation; i++)
		{
			AddRsvStations[i].type = FUNC_ADD;
			AddRsvStations[i].numOfRsvStationsFromThisType = pConfig->add_nr_reservation;
			snprintf(AddRsvStations[i].name, 10, "ADD%d", i + 1);
		}
		AddRsvStations->functionalUnits = safeCalloc(pConfig->add_nr_units, sizeof(FunctionUnit));
		for (i = 0; i < pConfig->add_nr_units; i++)
		{
			AddRsvStations->functionalUnits[i].delay = pConfig->add_delay;
			AddRsvStations->functionalUnits[i].numOfFunctionalUnitsFromThisType = pConfig->add_nr_units;
			AddRsvStations->functionalUnits[i].type = FUNC_ADD;
		}

		reservationStations[ADD] = AddRsvStations;
		reservationStations[SUB] = AddRsvStations;



		MulRsvStations = safeCalloc(pConfig->mul_nr_reservation, sizeof(RsvStation));
		for (i = 0; i < pConfig->mul_nr_reservation; i++)
		{
			MulRsvStations[i].type = FUNC_MULT;
			MulRsvStations[i].numOfRsvStationsFromThisType = pConfig->mul_nr_reservation;
			snprintf(MulRsvStations[i].name, 10, "MULT%d", i + 1);
		}
		MulRsvStations->functionalUnits = safeCalloc(pConfig->mul_nr_units, sizeof(FunctionUnit));
		for (i = 0; i < pConfig->mul_nr_units; i++)
		{
			MulRsvStations->functionalUnits[i].delay = pConfig->mul_delay;
			MulRsvStations->functionalUnits[i].numOfFunctionalUnitsFromThisType = pConfig->mul_nr_units;
			MulRsvStations->functionalUnits[i].type = FUNC_MULT;
		}
		reservationStations[MULT] = MulRsvStations;

		DivRsvStations = safeCalloc(pConfig->div_nr_reservation, sizeof(RsvStation));
		for (i = 0; i < pConfig->div_nr_reservation; i++)
		{
			DivRsvStations[i].type = FUNC_DIV;
			DivRsvStations[i].numOfRsvStationsFromThisType = pConfig->div_nr_reservation;
			snprintf(DivRsvStations[i].name, 10, "DIV%d", i + 1);
		}
		DivRsvStations->functionalUnits = safeCalloc(pConfig->div_nr_units, sizeof(FunctionUnit));
		for (i = 0; i < pConfig->div_nr_units; i++)
		{
			DivRsvStations->functionalUnits[i].delay = pConfig->div_delay;
			DivRsvStations->functionalUnits[i].numOfFunctionalUnitsFromThisType = pConfig->div_nr_units;
			DivRsvStations->functionalUnits[i].type = FUNC_DIV;

		}
		reservationStations[DIV] = DivRsvStations;

		LoadBuffers = safeCalloc(pConfig->mem_nr_load_buffers, sizeof(RsvStation));
		for (i = 0; i < pConfig->mem_nr_load_buffers; i++)
		{
			LoadBuffers[i].type = FUNC_LD;
			snprintf(LoadBuffers[i].name, 10, "LD%d", i + 1);
		}

		reservationStations[LD] = LoadBuffers;

		StoreBuffers = safeCalloc(pConfig->mem_nr_store_buffers, sizeof(RsvStation));
		for (i = 0; i < pConfig->mem_nr_store_buffers; i++)
		{
			StoreBuffers[i].type = FUNC_ST;
			snprintf(StoreBuffers[i].name, 10, "ST%d", i + 1);
		}
		reservationStations[ST] = StoreBuffers;


		//only one memory unit
		memoryUnit = safeCalloc(1, sizeof(FunctionUnit));
		memoryUnit->delay = pConfig->mem_delay;
		memoryUnit->numOfFunctionalUnitsFromThisType = 1;

	} while (FALSE);

	return status;
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

/************************************************************************/
/*                  MAIN       									        */
/************************************************************************/

int main(int argc, char** argv)
{
	time_t now, later;
	now = clock();
	STATUS			status = STATUS_SUCCESS;
	int				index, j, k;
	CONFIG			config = { 0 };
	PInstCtx		currentInst = NULL;
	PCHAR			opcode = NULL;
	pRsvStation		currentRsvStations;
	UINT32			numberOfRsvStations;
	BOOL			instructionIssued;

	pCDB			CDB;
	BOOL			wasHalt = FALSE;
	BOOL			isCPUreadyForHalt = TRUE;

	pFunctionUnit	FU_array;
	pFunctionUnit	pCurrentFU;
	pRsvStation		relevantReservationStations;
	pRsvStation		currentBuffer;

	do
	{
		printf("\n--- Tomsulo Algorithm Simulator ---\n\n");

		runCheckStatusBreak(FileParser_MeminParser, mem);
		printf("Parsed memin file successfully\n");

		runCheckStatusBreak(FileParser_ConfigParser, &config);
		printf("Parsed config file successfully\n");

		InitializeReservationsStations(&config);

		for (index = 0; index < NUM_REGS; index++)
		{
			F[index].value = (float)index;
			F[index].hasTag = FALSE;
			F[index].tagPC = 0;
		}
		PC = 0;

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
				status = Instructions_FetchTwoInstructions(pInstQ, mem, &PC);
				if (status != STATUS_SUCCESS && status != STATUS_QUEUE_FULL)
				{
					printf("[Instructions_FetchTwoInstructions] returned with status %d [PC = %d]\n", status, PC);
					break;
				}

				//if the first CC, cannot do anything yet because fetch takes full CC
				if (CC == 0)
				{
					CC++;
					continue;
				}


				/************************************************************************/
				/* Issue to reservation stations                                         */
				/************************************************************************/

				for (j = 0; j < 2; j++)
				{
					Queue_Peek(pInstQ, &currentInst);

					printf("\n** Working on instruction <PC=%d,OPCODE=%d>\n",
						currentInst->pc, currentInst->opcode);

					if (currentInst->opcode != HALT)
					{
						currentRsvStations = reservationStations[currentInst->opcode];

						numberOfRsvStations = OpcodeToNumberOfReservationStations(currentInst->opcode, &config);

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
								F[currentInst->DST].tagPC = currentInst->pc;
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
						wasHalt = TRUE;
						break;
					}
				}

			}


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
						breakpoint;
						isCPUreadyForHalt = FALSE;

						if (pCurrentFU->clockCycleCounter == pCurrentFU->delay)
						{
							printf("\nFuncUnit %s.%d finished\n", funame(j), index);

							pCurrentFU->pInstruction->cycleExecutionEnd = CC;
							//same index as the type of the functional unit,
							//try to write to CDB
							if (CDBs[j].tag == NULL)
							{
								//if relevant CDB is empty
								printf("FuncUnit %s.%d wrote value %f to CDB %d\n", funame(j), index, pCurrentFU->DST, j);

								//take tag from the instruction
								CDBs[j].tag = pCurrentFU->pInstruction->tag;
								CDBs[j].tagPC = pCurrentFU->pInstruction->pc;
								CDBs[j].value = pCurrentFU->DST;
								CDBs[j].CCupdated = CC;
								pCurrentFU->pInstruction->cycleWriteCDB = CC;

								safeFree(pCurrentFU->pInstruction);
								//finished with instruction :)

								pCurrentFU->clockCycleCounter = 0;
								pCurrentFU->busy = FALSE;
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
								breakpoint;

								//don't move instruction from reservation station to functional unit in the same CC
								if (relevantReservationStations[k].pInstruction->cycleIssued == CC)
									continue;

								isCPUreadyForHalt = FALSE;
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


			//pass on all the reservation station types
			for (j = 0; j < FUNC_CNT; j++)
			{
				if (j == LD || j == SUB) //LD is not relevant, SUB is the same as load
					continue;

				currentRsvStations = reservationStations[j];

				numberOfRsvStations = OpcodeToNumberOfReservationStations(j, &config);

				//all the reservation stations from this type
				for (index = 0; index < numberOfRsvStations; index++)
				{
					if (currentRsvStations[index].busy == TRUE)
					{
						breakpoint;
						printf("\nRsvStation %s has instruction PC=%d\n", currentRsvStations[index].name, currentRsvStations[index].pInstruction->pc);

						isCPUreadyForHalt = FALSE;
						//sanity check that the instruction in the reservation station was not
						//issued on this CC
						if (currentRsvStations[index].pInstruction->cycleIssued < CC)
						{
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




			/************************************************************************/
			/* PROCESS MEMORY UNIT													*/
			/************************************************************************/

			if (memoryUnit->busy == TRUE)
			{
				printf("\nmemoryUnit is BUSY with instruction PC = %d\n", memoryUnit->pInstruction->pc);
				breakpoint;
				isCPUreadyForHalt = FALSE;
				if (memoryUnit->clockCycleCounter == memoryUnit->delay)
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
						CDBs[j].tag = memoryUnit->pInstruction->tag;
						CDBs[j].value = memoryUnit->DST;

						memoryUnit->pInstruction->cycleWriteCDB = CC;

						safeFree(memoryUnit->pInstruction);
						//finished with instruction :)

						memoryUnit->clockCycleCounter = 0;
						memoryUnit->busy = FALSE;
					}
					//if CDBs[j].tag is not NULL then just wait, don't clock cycle counter
				}
				else
				{
					memoryUnit->clockCycleCounter++;
				}
			}
			else //memory unit is empty
			{
				pRsvStation buffers[2] = { LoadBuffers, StoreBuffers };

				for (j = 0; j < 2; j++)
				{
					currentBuffer = buffers[j];

					for (k = 0; k < currentBuffer->numOfRsvStationsFromThisType; k++)
					{
						if (currentBuffer[k].busy == TRUE)
						{
							breakpoint;
							isCPUreadyForHalt = FALSE;
							//relevant only for store, will always be NULL for load
							if (currentBuffer[k].Qk == NULL)
							{
								//if the store value is valid
								memoryUnit->busy = TRUE;

								memoryUnit->pInstruction = currentBuffer[k].pInstruction;

								memoryUnit->pInstruction->cycleExecutionStart = CC;

								currentBuffer[k].pInstruction = NULL;

								currentBuffer[k].busy = FALSE;

								switch (memoryUnit->pInstruction->opcode)
								{
								case LD:
									memoryUnit->DST = mem[memoryUnit->pInstruction->IMM];
									break;

								case ST:
									mem[memoryUnit->pInstruction->IMM] = currentBuffer[k].Vk;
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
						break; //break outer loop
				}
			}

			/************************************************************************/
			/*									Write Result						*/
			/************************************************************************/

			for (index = 0; index < NUM_REGS; index++)
			{
				if (F[index].hasTag == TRUE)
				{
					breakpoint;
					printf("\nRegister F[%d] has tag %s\n", index, F[index].tag->name);
					isCPUreadyForHalt = FALSE;
					for (k = 0; k < 4; k++) //pass on all CDB's
					{
						if (CDBs[k].tag != NULL)
						{
							//tag on CDB equals tag of register
							if (CDBs[k].tag == F[index].tag && CDBs[k].tagPC == F[index].tagPC)
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


			//Clear bus every CC because all needed was passed either to reservation station or register
			memset(CDBs, 0, sizeof(CDBs));

			if (wasHalt == TRUE && isCPUreadyForHalt == TRUE)
			{
				printf("\nHALT detected, BREAKING!!\n");
				breakpoint;
				break;
			}


			CC++;

		}

		/*safeFree(AddRsvStations->functionalUnits);
		safeFree(AddRsvStations);
		safeFree(MulRsvStations->functionalUnits);
		safeFree(MulRsvStations);
		safeFree(DivRsvStations->functionalUnits);
		safeFree(DivRsvStations);
		safeFree(LoadBuffers);
		safeFree(StoreBuffers);
		safeFree(memoryUnit);*/


	} while (FALSE);
	printf("\n\nFinal Register Values:\n^^^^^^^^^^^^^^^^^^^^^^^^\n\n");
	for (index = 0; index < NUM_REGS; index++)
		printf("F[%d] = %.1f\n", index, F[index].value);

	Queue_Destroy(pInstQ);
	cleanMemory();
	later = clock();

	printf("\n*** Memory Counter = %d\n", globalMemoryCounter);
	printf("\nProgram took %f ms\n", (double)(later - now) / 1000);
	printf("\npress any key to exit...\n");
	getchar();

	return 0;
}