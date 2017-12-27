#include <stdio.h>
#include <stdlib.h>
#include "FileParser.h"
#include "Queue.h"
#include "sim.h"
#include "Instructions.h"
#include "Instructions.h"
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

#define breakpoint			printf("## %s | %d\n",__FUNCTION__,__LINE__)

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
		AddRsvStations = safeCalloc(pConfig->add_nr_reservation , sizeof(RsvStation));
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



		MulRsvStations = safeCalloc(pConfig->mul_nr_reservation , sizeof(RsvStation));
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

		DivRsvStations = safeCalloc(pConfig->div_nr_reservation , sizeof(RsvStation));
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

		LoadBuffers = safeCalloc(pConfig->mem_nr_load_buffers , sizeof(RsvStation));
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
	STATUS		status = STATUS_SUCCESS;
	int			index,j,k;
	CONFIG		config = { 0 };
	PInstCtx	currentInst = NULL;
	PCHAR		opcode = NULL;
	pRsvStation currentRsvStations;
	UINT32		numberOfRsvStations;
	BOOL		instructionIssued;

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
		}
		PC = 0;

		runCheckStatusBreak(Queue_Create, &pInstQ, INSTRUCTION_QUEUE_LEN);

		while (TRUE)
		{
			printf("\n** CC = %d **\n", CC);

			/**
			 * Fetch up to two instructions and add them to the instructions queue
			 * if there's room.
			 */
			status = Instructions_FetchTwoInstructions(pInstQ, mem, &PC);
			if (status != STATUS_SUCCESS && status != STATUS_QUEUE_FULL)
			{
				printf("[Instructions_FetchTwoInstructions] returned with status %d [PC = %d]\n", status, PC);
				break;
			}

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
							//at this point can dequeue from isntruction queue
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
							//doesnt matter if has tag already
							F[currentInst->DST].hasTag = TRUE;
							F[currentInst->DST].tagType = currentRsvStations[index].type;
							F[currentInst->DST].tagIndex = index;

							if (currentInst->opcode != LD)
							{
								if (F[currentInst->SRC1].hasTag == FALSE)
								{
									currentRsvStations[index].Vk = F[currentInst->SRC1].value;
								}
								else
								{
									currentRsvStations[index].Qk = &reservationStations[F[currentInst->SRC1].tagType][F[currentInst->SRC1].tagIndex];
								}

								if (currentInst->opcode != ST)
								{
									if (F[currentInst->SRC0].hasTag == FALSE)
									{
										currentRsvStations[index].Vj = F[currentInst->SRC0].value;
									}
									else
									{
										currentRsvStations[index].Qj = &reservationStations[F[currentInst->SRC0].tagType][F[currentInst->SRC0].tagIndex];
									}
								}
							}

							break;
						}
						else  //currentRsvStations[index].busy is TRUE, there is instruction in reservation station
						{
							printf("\n** Station %s is BUSY\n", currentRsvStations[index].name);
							//check the relevant CDB if there's relevant information
							
							pCDB CDB;
							for (k = 0; k < 4; k++)
							{
								CDB = &CDBs[k];

								if (currentRsvStations[index].Qj == CDB->tag)
								{
									CDB->tag = NULL;
									currentRsvStations[index].Qj = NULL;
									currentRsvStations[index].Vj = CDB->value;
								}
								else if (currentRsvStations[index].Qk == CDB->tag)
								{
									CDB->tag = NULL;
									currentRsvStations[index].Qk = NULL;
									currentRsvStations[index].Vk = CDB->value;
								}
							}
							
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
					break;
				}
			}

			pFunctionUnit FUs[3] = { AddRsvStations->functionalUnits,
									MulRsvStations->functionalUnits,
									DivRsvStations->functionalUnits };
			pFunctionUnit	FU_array;
			pFunctionUnit	pCurrentFU;
			pRsvStation		relevantReservationStations;
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
						if (pCurrentFU->clockCycleCounter == pCurrentFU->delay)
						{
							pCurrentFU->pInstruction->cycleExecutionEnd = CC;
							//same index as the type of the functional unit,
							//try to write to CDB
							if (CDBs[j].tag == NULL)	
							{
								//if relevant CDB is empty

								//take tag from the instruction
								CDBs[j].tag = pCurrentFU->pInstruction->tag;
								CDBs[j].value = pCurrentFU->DST;

								pCurrentFU->pInstruction->cycleWriteCDB = CC;

								pCurrentFU->clockCycleCounter = 0;
								pCurrentFU->busy = FALSE;
							}
							//if CDBs[j].tag is not NULL then just wait, don't clock cycle counter
						}
						else
						{
							pCurrentFU->clockCycleCounter++;
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
								if (relevantReservationStations[k].Qj == NULL &&
									relevantReservationStations[k].Qk == NULL)
								{

									//if they are both empty, namely, if values are relevant in reservation station
									pCurrentFU->busy = TRUE;

									pCurrentFU->pInstruction = relevantReservationStations[k].pInstruction;

									pCurrentFU->pInstruction->cycleExecutionStart = CC;

									relevantReservationStations[k].pInstruction = NULL;

									pCurrentFU->SRC0 = relevantReservationStations[k].Vj;
									pCurrentFU->SRC1 = relevantReservationStations[k].Vk;
									relevantReservationStations[k].busy = FALSE;
									////////
									printf("\tpCurrentFU->pInstruction = %p\n", pCurrentFU->pInstruction);
									// Do the actual calculation and save the result inside the functional unit
									switch (pCurrentFU->pInstruction->opcode)
									{
									case ADD:

										pCurrentFU->DST = pCurrentFU->SRC0 + pCurrentFU->SRC1;

										break;
									case SUB:

										pCurrentFU->DST = pCurrentFU->SRC0 - pCurrentFU->SRC1;
										printf("\t\tSUB happened - DST = %f\n", pCurrentFU->DST);

										break;
									case MULT:

										pCurrentFU->DST = pCurrentFU->SRC0 * pCurrentFU->SRC1;

										break;
									case DIV:

										pCurrentFU->DST = pCurrentFU->SRC0 / pCurrentFU->SRC1;

										break;
									default:
										printf("WRONG INSTRUCTION\n");
										break;
									}
									////////

									break;
								}
								else //Qk or Qj waiting for result
								{
									//do nothing for now
								}
							}	
						}
					}
				}
			}
			
			// Process memory unit

			if (currentInst->opcode == HALT)
				break;

			CC++;
			Sleep(100);	//to show progress in cmd
		}

		safeFree(AddRsvStations->functionalUnits);
		safeFree(AddRsvStations);
		safeFree(MulRsvStations->functionalUnits);
		safeFree(MulRsvStations);
		safeFree(DivRsvStations->functionalUnits);
		safeFree(DivRsvStations);
		safeFree(LoadBuffers);
		safeFree(StoreBuffers);
		safeFree(memoryUnit);

		Queue_Destroy(pInstQ);

	} while (FALSE);

	printf("*** Memory Counter = %d\n", globalMemoryCounter);
	printf("\npress any key to exit...\n");
	getchar();

	return 0;
}