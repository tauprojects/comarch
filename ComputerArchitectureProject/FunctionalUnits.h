#ifndef FUNCTIONALUNITS_H_
#define FUNCTIONALUNITS_H_

#include "mainDefs.h"

/**
 * This module uses the global variables as externs so no need to pass pointers to every function.
 */
extern UINT32			mem[];

extern pRsvStation		reservationStations[];
extern pRsvStation		AddRsvStations;
extern pRsvStation		MulRsvStations;
extern pRsvStation		DivRsvStations;
extern pRsvStation		LoadBuffers;
extern pRsvStation		StoreBuffers;

extern UINT32			CC;
extern _CDB				CDBs[];
/************************************************************************/




/**
* This function initializes the global memory unit based on the config.
* The memory unit has "delay" cells - this is the implementation of the pipelining of the memory unit.
*
* @param pConfig	-	pointer to configuration
*/
VOID CPU_InitializeMemoryUnit(PCONFIG pConfig);




/**
* This function does the entire processing for the ADD,MUL,DIV functional units -
* using #CPU_AddInstructionToFunctionalUnit if the unit is empty,
* and #CPU_ProcessInstructionInFunctionalUnit if the unit is not empty.
*
* @param pisCPUreadyForHalt	-	pointer to a boolean indicating whether something has happenned, and we cannot
*									halt in this CC
*/
VOID CPU_ProcessFunctionalUnits(PBOOL pisCPUreadyForHalt);




/**
* This function does all the processing of the memory unit in each clock cycle.
* It passes on all the load and store buffers, and if one is empty it searches for the first available
* memory instruction that can be executed (it's address is not being used by another instruction.
* The pipelining is implemented by using the same basic structure of an array of functional units
* (which are the same as memory units), but that only one instruction can be issued every clock cycle.
*
* If a memory unit cell is currently processing an instruction, its delay is incremented. If the delay
* has ended, it writes / reads the memory IN THE LAST CC (both in the last CC), and broadcasts to the MEM CDB
* if the CDB is now empty.
*
* @param	pIsCPUreadyForHalt	-	pointer to boolean indicating whether something happened in this CC.
*									if it did, then halt must be issued only in the next CC.
*/
VOID CPU_ProcessMemoryUnit(PBOOL pIsCPUreadyForHalt);




#endif FUNCTIONALUNITS_H_