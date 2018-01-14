
#include "FunctionalUnits.h"
#include <stdio.h>
#include <stdlib.h>

/* GLOBAL OF memory Unit */
pFunctionUnit	memoryUnit;


/************************************************************************/
/* Internal Functions                                                   */
/************************************************************************/

/**
 * This function cleans a functional unit, and zeroes the counter.
 */
static VOID CPU_CleanFU(pFunctionUnit fu)
{
	if (fu)
	{
		if (fu->pInstruction)
		{
			if (fu->pInstruction->tag)
			{
				fu->pInstruction->tag->pInstruction = NULL; 
				fu->pInstruction->tag->busy = FALSE;
			}
		}

		fu->clockCycleCounter = 0;
		fu->busy = FALSE;
	}
}

/**
 * This function processes an instruction in the functional unit.
 * If the delay has not ended, it increments the counter, 
 * If it has ended it tries to broadcast it to a CDB.
 */
static VOID CPU_ProcessInstructionInFunctionalUnit(pFunctionUnit pCurrentFU, UINT32 FUtype)
{
	// if the functional unit finished working on the instruction
	// delay-1 because the 0 CC in the count counts also
	if (pCurrentFU->clockCycleCounter == pCurrentFU->delay - 1)
	{
		dprintf("\nFuncUnit %s finished\n", pCurrentFU->name);

		if (pCurrentFU->pInstruction->cycleExecutionEnd == 0)
		{
			pCurrentFU->pInstruction->cycleExecutionEnd = CC;
		}
		else
		{ 
			//it didn't end in this CC - broadcast 1 CC after finish of instruction

			//same index as the type of the functional unit - ADD / MUL / DIV (no memory here)
			//try to write to CDB
			if (CDBs[FUtype].tag == NULL)
			{
				//if relevant CDB is empty
				dprintf("FuncUnit %s wrote value %f to CDB %d\n", pCurrentFU->name, pCurrentFU->DST, pCurrentFU->type);

				//take tag from the instruction
				CDBs[FUtype].tag = pCurrentFU->pInstruction->tag;
				CDBs[FUtype].inst = pCurrentFU->pInstruction;
				CDBs[FUtype].value = pCurrentFU->DST;
				pCurrentFU->pInstruction->cycleWriteCDB = CC;

				CPU_CleanFU(pCurrentFU);
			}

		}
		//if CDBs[j].tag is not NULL then just wait, don't clock cycle counter
	}
	else
	{
		pCurrentFU->clockCycleCounter++;

		dprintf("\nFuncUnit %s counter = %d\n", pCurrentFU->name, pCurrentFU->clockCycleCounter);
	}
}

/**
 * This function does the actual value calculation based on the opcode (not memory operations).
 */
static VOID CPU_CalculateResult(pFunctionUnit pCurrentFU)
{
	switch (pCurrentFU->pInstruction->opcode)
	{
	case ADD:

		pCurrentFU->DST = pCurrentFU->SRC0 + pCurrentFU->SRC1;
		dprintf("\nADD happened - DST = %f\n", pCurrentFU->DST);
		break;

	case SUB:

		pCurrentFU->DST = pCurrentFU->SRC0 - pCurrentFU->SRC1;
		dprintf("\nSUB happened - DST = %f\n", pCurrentFU->DST);
		break;

	case MULT:

		pCurrentFU->DST = pCurrentFU->SRC0 * pCurrentFU->SRC1;
		dprintf("\nMULT happened - DST = %f\n", pCurrentFU->DST);
		break;

	case DIV:

		pCurrentFU->DST = pCurrentFU->SRC0 / pCurrentFU->SRC1;
		dprintf("\nDIV happened - DST = %f\n", pCurrentFU->DST);
		break;

	default:
		dprintf("\nWRONG INSTRUCTION\n");
		break;
	}
}

/**
 * This function moves an instruction from the reservation station to the functional unit.
 */
static VOID CPU_AddInstructionToFunctionalUnit(pFunctionUnit pCurrentFU, PBOOL pisCPUreadyForHalt)
{
	pRsvStation		rsvStationArray = reservationStations[pCurrentFU->type];
	pRsvStation		pCurrentRsvSt;
	UINT32			k;

	//check reservations stations from this type and take first valid instruction
	for (k = 0; k < rsvStationArray->numOfRsvStationsFromThisType; k++)
	{
		pCurrentRsvSt = &rsvStationArray[k];

		if (pCurrentRsvSt->busy == TRUE &&
			pCurrentRsvSt->isInstInFuncUnit == FALSE)
		{

			//don't move instruction from reservation station to functional unit in the same CC as it was issued
			if (pCurrentRsvSt->pInstruction->cycleIssued == CC)
				continue;

			*pisCPUreadyForHalt = FALSE;

			if (pCurrentRsvSt->Qj == NULL && pCurrentRsvSt->Qk == NULL)
			{
				dprintf("\nFuncUnit %s taking PC %d from RsvStation %s\n", pCurrentFU->name, pCurrentRsvSt->pInstruction->pc, pCurrentRsvSt->name);

				//to prevent other functional units from taking this instruction even though
				//the station was busy
				pCurrentRsvSt->isInstInFuncUnit = TRUE;

				//if they are both empty, namely, if values are relevant in reservation station
				pCurrentFU->busy = TRUE;

				pCurrentFU->pInstruction = pCurrentRsvSt->pInstruction;

				pCurrentFU->pInstruction->cycleExecutionStart = CC;

				pCurrentFU->SRC0 = pCurrentRsvSt->Vj;
				pCurrentFU->SRC1 = pCurrentRsvSt->Vk;

				pCurrentFU->clockCycleCounter++; //this cycle counts

				// Do the actual calculation and save the result inside the functional unit
				CPU_CalculateResult(pCurrentFU);

				break;
			}
		}
	}
}

/**
 * This function is used for memory instructions. It checks whether all the following conditions occur:
 * 1. currently an instruction being processes in the memory unit
 * 2. that this instruction has the same address as in the input buffer
 * 3. this instruction depends on the found instruction (its PC is larger)
 * 4. Either of the following:
 *		- The first instruction is LD and the second is ST, then we must wait for the first instruction to finish.
 *		- The address is in the buffer, but has not yet started processing in the memory unit, so we must wait.
 * If all the conditions follow, we know we must wait for the first instruction to finish.
 */
static BOOL CPU_CheckIfAddressInRsvSta(pRsvStation currentMemBuffer)
{
	pRsvStation buffers[2] = { LoadBuffers, StoreBuffers };
	UINT32		j, k;

	//for every buffer type
	for (j = 0; j < 2; j++)
	{
		//for every buffer from this type
		for (k = 0; k < buffers[j]->numOfRsvStationsFromThisType; k++)
		{
			if (&buffers[j][k] != currentMemBuffer &&															//not the same buffer
				buffers[j][k].busy == TRUE &&																	//buffer has instruction
				currentMemBuffer->pInstruction->IMM == buffers[j][k].Address &&									//Address is the same 
				currentMemBuffer->pInstruction->pc > buffers[j][k].pInstruction->pc &&							//PC of the current is bigger
				((currentMemBuffer->pInstruction->opcode == ST && buffers[j][k].pInstruction->opcode == LD) ||	//first is LD and second is ST
					buffers[j][k].isInstInFuncUnit == FALSE)													//instruction not in mem unit yet
				)
			{
				dprintf("\n** buffer %s found address <%d> in RsvSta %s, WAITING\n", currentMemBuffer->name, buffers[j][k].Address, buffers[j][k].name);
				return TRUE;
			}
		}
	}

	return FALSE;
}

/**
 * This function checks if the instruction that has just finished in the memory unit can pass its address
 * to another reservation station that is waiting for it. The conditions are inside the function notes.
 */
static VOID CPU_PassAddressToRsvSta(PInstCtx pCurrentInst)
{
	pRsvStation buffers[2] = { LoadBuffers, StoreBuffers };
	UINT32		j, k;
	BOOL		found = FALSE;

	for (j = 0; j < 2; j++)
	{
		for (k = 0; k < buffers[j]->numOfRsvStationsFromThisType; k++)
		{
			if (&buffers[j][k] != pCurrentInst->tag &&		//not the same buffer
				buffers[j][k].busy == TRUE &&				//buffer has instruction
				buffers[j][k].Address == 0 &&				//its waiting for address from somewhere else (based on #CPU_CheckIfAddressInRsvSta)
				buffers[j][k].isInstInFuncUnit == FALSE)	//sanity check
			{
				//another if to prevent null dereference if pInstruction is NULL
				if (buffers[j][k].pInstruction->IMM == pCurrentInst->IMM &&			//both instruction has the same address
					buffers[j][k].pInstruction->pc > pCurrentInst->pc)				//waiting instruction's PC is bigger (depends on first instruction)
				{
					buffers[j][k].Address = pCurrentInst->IMM;						//if so - give him the address
				}
			}
		}
	}
}




/************************************************************************/
/* Public Functions	                                                    */
/************************************************************************/

VOID CPU_InitializeMemoryUnit(PCONFIG pConfig)
{
	UINT32	i;
	UINT32	len = pConfig->mem_delay;

	//Pipeline is in the length of the delay
	memoryUnit = safeCalloc(len, sizeof(FunctionUnit));
	memoryUnit->numOfFunctionalUnitsFromThisType = len;

	for (i = 0; i < len; i++)
	{
		memoryUnit[i].delay = pConfig->mem_delay;
	}
}

VOID CPU_ProcessFunctionalUnits(PBOOL pisCPUreadyForHalt)
{
	UINT32			fuIndex, fuType;
	pFunctionUnit	pCurrentFU;
	pFunctionUnit	FUs[3] = { AddRsvStations->functionalUnits,
								MulRsvStations->functionalUnits,
								DivRsvStations->functionalUnits };


	//pass on every type of functional unit
	for (fuType = 0; fuType < 3; fuType++)
	{

		//pass on every functional unit from the specific type
		for (fuIndex = 0; fuIndex < FUs[fuType]->numOfFunctionalUnitsFromThisType; fuIndex++)
		{
			pCurrentFU = &FUs[fuType][fuIndex];

			if (pCurrentFU->busy == TRUE)
			{
				*pisCPUreadyForHalt = FALSE;

				CPU_ProcessInstructionInFunctionalUnit(pCurrentFU, fuType);
			}
			else
			{
				CPU_AddInstructionToFunctionalUnit(pCurrentFU, pisCPUreadyForHalt);
			}
		}
	}
}

VOID CPU_ProcessMemoryUnit(PBOOL pIsCPUreadyForHalt)
{
	UINT32			i, j, k;
	pRsvStation		currentBuffer;
	pRsvStation		buffers[2] = { LoadBuffers, StoreBuffers };
	pFunctionUnit	curMemUnit;
	BOOL			wasInstructedIssuedInThisCC = FALSE;
	
	for (i = 0; i < memoryUnit->numOfFunctionalUnitsFromThisType; i++)
	{
		curMemUnit = &memoryUnit[i];

		if (curMemUnit->busy == TRUE)
		{
			dprintf("\nmemoryUnit[%d] is BUSY with instruction PC = %d\n", i, curMemUnit->pInstruction->pc);
			*pIsCPUreadyForHalt = FALSE;

			//instruction has ended processing
			if (curMemUnit->clockCycleCounter == curMemUnit->delay - 1)
			{
				dprintf("\nmemoryUnit[%d] is finished instruction PC = %d\n", i,curMemUnit->pInstruction->pc);


				if (curMemUnit->pInstruction->opcode == ST)
				{
					//actual write to memory, in the last CC of operation
					mem[curMemUnit->pInstruction->tag->Address] = Float2Hex(curMemUnit->DST);
					dprintf("\nMemoryUnit[%d] wrote value %f <%X> to MEM[%d]\n", i, curMemUnit->DST, mem[curMemUnit->pInstruction->tag->Address], curMemUnit->pInstruction->tag->Address);
				}
				else // Load instruction
				{
					//actual load from memory, in the last CC of operation
					curMemUnit->DST = Hex2Float(mem[curMemUnit->pInstruction->tag->Address]);
					dprintf("\nMemoryUnit[%d] read value %f <%X> from MEM[%d]\n", i, curMemUnit->DST, mem[curMemUnit->pInstruction->tag->Address], curMemUnit->pInstruction->tag->Address);
				}


				if (curMemUnit->pInstruction->cycleExecutionEnd == 0)
				{
					curMemUnit->pInstruction->cycleExecutionEnd = CC;
				}
				else //it didn't end in this CC
				{ 

					CPU_PassAddressToRsvSta(curMemUnit->pInstruction);

					//same index as the type of the functional unit,
					//try to write to CDB
					
					if (curMemUnit->pInstruction->opcode == ST)
					{
						curMemUnit->pInstruction->cycleWriteCDB = -1;

						CPU_CleanFU(curMemUnit);				
					}
					else // Load instruction
					{
						if (CDBs[3].tag == NULL)
						{
							//if relevant CDB is empty
							dprintf("\nMemoryUnit[%d] wrote value %f to CDB 3\n", i, curMemUnit->DST);

							//take tag from the instruction
							CDBs[3].tag = curMemUnit->pInstruction->tag;
							CDBs[3].inst = curMemUnit->pInstruction;
							CDBs[3].value = curMemUnit->DST;
							curMemUnit->pInstruction->cycleWriteCDB = CC;

							CPU_CleanFU(curMemUnit);

						}
						//if CDBs[3].tag is not NULL then just wait, don't increment clock cycle counter
					}

					//Clear memoryUnit



				}
			}
			else //memory unit hasn't finished
			{
				curMemUnit->clockCycleCounter++;
				dprintf("\nMemoryUnit[%d] counter = %d\n", i, curMemUnit->clockCycleCounter);

			}
		}
		else if(wasInstructedIssuedInThisCC == FALSE)//memory unit is empty but no instruction issued already
		{
			for (j = 0; j < 2; j++)
			{
				currentBuffer = buffers[j];

				for (k = 0; k < currentBuffer->numOfRsvStationsFromThisType; k++)
				{
					if (currentBuffer[k].busy == TRUE &&
						currentBuffer[k].isInstInFuncUnit == FALSE &&
						CPU_CheckIfAddressInRsvSta(&currentBuffer[k]) == FALSE)	//if its not waiting for another instruction with the same memory address
					{

						//don't move instruction from reservation station to memory unit in the same CC
						if (currentBuffer[k].pInstruction->cycleIssued == CC)
							continue;

						*pIsCPUreadyForHalt = FALSE;
						//relevant only for store, will always be NULL for load
						if (currentBuffer[k].Qk == NULL)
						{
							dprintf("\nMemoryUnit[%d] taking PC %d from RsvStation %s\n", i, currentBuffer[k].pInstruction->pc, currentBuffer[k].pInstruction->tag->name);

							wasInstructedIssuedInThisCC = TRUE;
							currentBuffer[k].isInstInFuncUnit = TRUE;
							//if the store value is valid
							curMemUnit->busy = TRUE;

							curMemUnit->pInstruction = currentBuffer[k].pInstruction;

							curMemUnit->pInstruction->cycleExecutionStart = CC;
							curMemUnit->clockCycleCounter++; //this cycle counts

							switch (curMemUnit->pInstruction->opcode)
							{
							case LD:
								break;

							case ST:
								curMemUnit->DST = currentBuffer[k].Vk;
								break;

							default:
								dprintf("WRONG INSTRUCTION\n");
								break;
							}

							break;
						}
					}
					
					if (wasInstructedIssuedInThisCC == TRUE)
						break;		//issue only one instruction per CC
				}
				
				if (wasInstructedIssuedInThisCC == TRUE)
					break;		//issue only one instruction per CC		
				//don't break outer loop because other instruction may still need CC++
			}
		}
	}
}