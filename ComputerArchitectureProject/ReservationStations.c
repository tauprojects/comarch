
#include "ReservationStations.h"
#include <stdlib.h>
#include <stdio.h>



/************************************************************************/
/* Internal Functions                                                   */
/************************************************************************/

static VOID RsvSta_InitializeOneRsvStation(pRsvStation* pRsvStationsTypeArray, UINT32 nr_reservation,
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

	//Initialize functional unit
	if (type != LD && type != ST)
	{
		RsvStationsTypeArray->functionalUnits = safeCalloc(nr_units, sizeof(FunctionUnit));
		for (i = 0; i < nr_units; i++)
		{
			RsvStationsTypeArray->functionalUnits[i].delay = delay;
			RsvStationsTypeArray->functionalUnits[i].numOfFunctionalUnitsFromThisType = nr_units;
			RsvStationsTypeArray->functionalUnits[i].type = type;
			snprintf(RsvStationsTypeArray->functionalUnits[i].name, 10, "%s%d", name, i);

		}
	}
}

static UINT32 RsvSta_OpcodeToNumberOfRsvStations(eInstType opcode, PCONFIG config)
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
/* Public Functions	                                                    */
/************************************************************************/


VOID RsvSta_InitializeReservationsStations(PCONFIG pConfig)
{
	RsvSta_InitializeOneRsvStation(&AddRsvStations, pConfig->add_nr_reservation, "ADD",
		pConfig->add_nr_units, ADD, pConfig->add_delay);

	reservationStations[SUB] = AddRsvStations;	//so SUB instructions would go to ADD rsv stations also

	RsvSta_InitializeOneRsvStation(&MulRsvStations, pConfig->mul_nr_reservation, "MUL",
		pConfig->mul_nr_units, MULT, pConfig->mul_delay);

	RsvSta_InitializeOneRsvStation(&DivRsvStations, pConfig->div_nr_reservation, "DIV",
		pConfig->div_nr_units, DIV, pConfig->div_delay);

	RsvSta_InitializeOneRsvStation(&LoadBuffers, pConfig->mem_nr_load_buffers, "LD",
		0, LD, 0);

	RsvSta_InitializeOneRsvStation(&StoreBuffers, pConfig->mem_nr_store_buffers, "ST",
		0, ST, 0);
}

/**
 * This function prevents accessing the result of a memory instruction while it is in operation,
	when the current instruction has the same memory address as the one in the memory buffer.
	Namely, the 2nd instruction should wait after the first is done.
	Example:

	LD F5 40
	ST F7 40
 */
static VOID RsvSta_FillInAddress(pRsvStation currentBuffer, PInstCtx pCurrentInst)
{
	pRsvStation buffers[2] = { LoadBuffers, StoreBuffers };
	UINT32		j,k;
	BOOL		found = FALSE;

	for (j = 0; j < 2; j++)
	{
		for (k = 0; k < buffers[j]->numOfRsvStationsFromThisType; k++)
		{
			if (&buffers[j][k] != currentBuffer &&								//not the same buffer
				buffers[j][k].busy == TRUE)										//buffer has instruction in it
			{
				//another if to prevent null dereferencing of pInstruction
				if (pCurrentInst->IMM == buffers[j][k].pInstruction->IMM &&		//both instruction has the same address (IMM)
					pCurrentInst->pc > buffers[j][k].pInstruction->pc &&		//current instruction came after the one now in buffer, second one depends on first
					pCurrentInst->opcode == ST && buffers[j][k].pInstruction->opcode == LD)	//wait for result only if 2nd is store and first is load
				{
					dprintf("\nRsvStation %s found address %d on buffer %s\n", currentBuffer->name, pCurrentInst->IMM, buffers[j][k].name);
					currentBuffer->Address = 0;
					found = TRUE;
					break;
				}
			}
		}
		if (found == TRUE)
		{
			break;
		}
	}

	if (found == FALSE)
	{
		currentBuffer->Address = pCurrentInst->IMM;
	}
}

VOID RsvSta_IssueInstToRsvStations(PCONFIG pConfig, PBOOL pWasHolt, PQUEUE pInstQ, UINT32 CC)
{
	UINT32		index, j, numberOfRsvStations;
	BOOL		instructionIssued;
	pRsvStation rsvStArray;
	PInstCtx	currentInst;

	for (j = 0; j < 2; j++)
	{
		Queue_Peek(pInstQ, &currentInst);

		dprintf("\n** Working on instruction <PC=%d,OPCODE=%d>\n",
			currentInst->pc, currentInst->opcode);
		if (currentInst->opcode != HALT)
		{
			rsvStArray = reservationStations[currentInst->opcode];
			numberOfRsvStations = RsvSta_OpcodeToNumberOfRsvStations(currentInst->opcode, pConfig);
			instructionIssued = FALSE;

			for (index = 0; index < numberOfRsvStations; index++)
			{
				if (rsvStArray[index].busy == FALSE)
				{
					//Found empty reservation station
					//at this point can dequeue from instruction queue
					Queue_Dequeue(pInstQ, &currentInst);

					currentInst->cycleIssued = CC;

					currentInst->tag = &rsvStArray[index];
					// insert instruction to reservation station
					rsvStArray[index].pInstruction = currentInst;
					rsvStArray[index].busy = TRUE;
					rsvStArray[index].isInstInFuncUnit = FALSE;
					instructionIssued = TRUE;

					FilesManager_AddToIssueArray(currentInst);

					dprintf("\n** Added instruction <PC=%d,OPCODE=%d> to station %s\n",
						currentInst->pc, currentInst->opcode, rsvStArray[index].name);

					//Update destination tag in register
					//doesn't matter if has tag already

					if (currentInst->opcode != LD)
					{
						if (F[currentInst->SRC1].hasTag == FALSE)
						{
							rsvStArray[index].Vk = F[currentInst->SRC1].value;
						}
						else
						{
							rsvStArray[index].Qk = F[currentInst->SRC1].tag;
							dprintf("\n RsvStation %s Qk took tag from F[%d] = %s\n", rsvStArray[index].name, currentInst->SRC1, F[currentInst->SRC1].tag->name);
						}

						if (currentInst->opcode != ST)
						{
							if (F[currentInst->SRC0].hasTag == FALSE)
							{
								rsvStArray[index].Vj = F[currentInst->SRC0].value;
							}
							else
							{
								rsvStArray[index].Qj = F[currentInst->SRC0].tag;
								dprintf("\n RsvStation %s Qj took tag from F[%d] = %s\n", rsvStArray[index].name, currentInst->SRC0, F[currentInst->SRC0].tag->name);
							}
						}
						else //instruction is STORE
						{
							RsvSta_FillInAddress(&rsvStArray[index], currentInst);
						}
					}
					else //instruction is LOAD
					{
						RsvSta_FillInAddress(&rsvStArray[index], currentInst);
					}

					//if instruction isn't store, add tag to register
					if (currentInst->opcode != ST)
					{
						F[currentInst->DST].hasTag = TRUE;
						F[currentInst->DST].tag = &rsvStArray[index];
						F[currentInst->DST].inst = currentInst;
						dprintf("\nWrote tag %s to register F[%d] \n", F[currentInst->DST].tag->name, currentInst->DST);
					}

					break;
				}
				else
				{
					dprintf("\n** Station %s is BUSY\n", rsvStArray[index].name);
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
			dprintf("Instruction (PC = %d) is HALT\n", currentInst->pc);
			*pWasHolt = TRUE;
			break;
		}
	}
}


VOID RsvSta_CheckIfRsvStationCanGetDataFromCDB(PBOOL pIsCPUreadyForHalt, PCONFIG pConfig)
{
	UINT32		j, index, k, numberOfRsvStations;
	pRsvStation rsvStArray, pCurrentRsvSt;
	pCDB		CDB;

	//pass on all the reservation station types
	for (j = 0; j < FUNC_CNT; j++)
	{
		if (j == LD || j == SUB) //LD is not relevant, SUB is the same as ADD
			continue;

		rsvStArray = reservationStations[j];

		numberOfRsvStations = RsvSta_OpcodeToNumberOfRsvStations(j, pConfig);

		//all the reservation stations from this type
		for (index = 0; index < numberOfRsvStations; index++)
		{
			pCurrentRsvSt = &rsvStArray[index];

			if (pCurrentRsvSt->busy == TRUE)
			{
				dprintf("\nRsvStation %s has instruction PC=%d\n", pCurrentRsvSt->name, pCurrentRsvSt->pInstruction->pc);

				*pIsCPUreadyForHalt = FALSE;

				for (k = 0; k < NUM_CDBS; k++)	//number of CDBs
				{
					//check the relevant CDB if there's relevant information

					CDB = &CDBs[k];

					if (CDB->tag != NULL)
					{
						if (pCurrentRsvSt->Qj == CDB->tag)
						{
							dprintf("\nRsvStation %s took value Vj from CDB %d for PC=%d\n", pCurrentRsvSt->name, k, pCurrentRsvSt->pInstruction->pc);
							pCurrentRsvSt->Qj = NULL;
							pCurrentRsvSt->Vj = CDB->value;
						}
						else if (pCurrentRsvSt->Qk == CDB->tag)
						{
							dprintf("\nRsvStation %s took value Vk from CDB %d for PC=%d\n", pCurrentRsvSt->name, k, pCurrentRsvSt->pInstruction->pc);
							pCurrentRsvSt->Qk = NULL;
							pCurrentRsvSt->Vk = CDB->value;
						}
					}
				}

			}
		}
	}
}