
#include "FunctionalUnits.h"
#include <stdlib.h>


/************************************************************************/
/* Internal Functions                                                   */
/************************************************************************/

static VOID CPU_ProcessInstructionInFunctionalUnit(pFunctionUnit pCurrentFU, UINT32 j)
{
	// if the functional unit finished working on the instruction
	// delay-1 because the 0 CC in the count counts also
	if (pCurrentFU->clockCycleCounter == pCurrentFU->delay - 1)
	{
		printf("\nFuncUnit %s finished\n", pCurrentFU->name);

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
				printf("FuncUnit %s wrote value %f to CDB %d\n", pCurrentFU->name, pCurrentFU->DST, pCurrentFU->type);

				//take tag from the instruction
				CDBs[j].tag = pCurrentFU->pInstruction->tag;
				CDBs[j].inst = pCurrentFU->pInstruction;
				CDBs[j].value = pCurrentFU->DST;
				CDBs[j].CCupdated = CC;
				pCurrentFU->pInstruction->cycleWriteCDB = CC;

				pCurrentFU->pInstruction->tag->pInstruction = NULL; //clean reservation station
				pCurrentFU->pInstruction->tag->busy = FALSE;

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
		printf("\nFuncUnit %s counter = %d\n", pCurrentFU->name, pCurrentFU->clockCycleCounter);
	}
}

static VOID CPU_CalculateResult(pFunctionUnit pCurrentFU)
{
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
}

static VOID CPU_AddInstructionToFunctionalUnit(pFunctionUnit pCurrentFU, PBOOL pisCPUreadyForHalt)
{
	pRsvStation		rsvStationArray = reservationStations[pCurrentFU->type];
	pRsvStation		pCurrentRsvSt;
	UINT32			k;

	//check reservations stations from this type and take first valid instruction
	for (k = 0; k < rsvStationArray->numOfRsvStationsFromThisType; k++)
	{
		pCurrentRsvSt = &rsvStationArray[k];

		if (pCurrentRsvSt->busy == TRUE
			&& pCurrentRsvSt->isInstInFuncUnit == FALSE)
		{

			//don't move instruction from reservation station to functional unit in the same CC
			if (pCurrentRsvSt->pInstruction->cycleIssued == CC)
				continue;

			*pisCPUreadyForHalt = FALSE;

			if (pCurrentRsvSt->Qj == NULL && pCurrentRsvSt->Qk == NULL)
			{
				printf("\nFuncUnit %s taking PC %d from RsvStation %s\n", pCurrentFU->name, pCurrentRsvSt->pInstruction->pc, pCurrentRsvSt->name);

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

/************************************************************************/
/* Public Functions	                                                    */
/************************************************************************/

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

