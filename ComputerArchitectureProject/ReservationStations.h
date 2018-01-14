#ifndef RESERVATIONSTATIONS_H_
#define RESERVATIONSTATIONS_H_

#include "Queue.h"

/**
* This module uses the global variables as externs so no need to pass pointers to every function.
*/
extern pRsvStation		reservationStations[];
extern pRsvStation		AddRsvStations;
extern pRsvStation		MulRsvStations;
extern pRsvStation		DivRsvStations;

extern pRsvStation		LoadBuffers;
extern pRsvStation		StoreBuffers;

extern _CDB				CDBs[];
extern Register			F[];
/************************************************************************/

/**
* This function initializes all reservation stations using the function #RsvSta_InitializeReservationsStations
* Each reservation station is initialized with its corresponding parameters from the configurations.
*
* @param	pConfig - pointer to the config file
* @see		RsvSta_InitializeReservationsStations
*/
VOID RsvSta_InitializeReservationsStations(PCONFIG pConfig);

/**
* This function checks the head of the instruction queue, and according to its opcode,
* it checks whether there's an empty reservation station from the corresponding type.
* The instruction is then inserted to the FIRST empty reservation station, if one exists.
* and the instruction is dequeued from the instruction queue.
* If insertion succeeds, it then repeats the process for a second instruction.
* If it doesn't (namely, there is no empty reservation station),
* or if the first instruction was a HALT instruction, the function does not dequeue
* a second instruction from the queue and does not repeat the process.
*
* @param	pConfig		-	pointer to configuration
* @param	pWasHalt	-	pointer to boolean indicating whether a HALT instruction was issued
* @param	pInstQ		-	pointer to the instruction queue
* @param	CC			-	current clock cycle to update cycleIssued parameter in instruction
*/
VOID RsvSta_IssueInstToRsvStations(PCONFIG pConfig, PBOOL pWasHolt, PQUEUE pInstQ, UINT32 CC);

/**
* This function checks whether some reservation stations has the same tag as currently broadcasted
* by the CDBs. The function checks all CDBs for all stations from all the types.
* If it finds a match, the value is copied from the CDB to the reservation station, and the tag is deleted
* from the station.
*
* @param	pIsCPUreadyForHalt	-	pointer to boolean indicating whether something happened in this CC.
*									if it did, then halt must be issued only in the next CC.
* @param	pConfig				-	pointer to the configuration
*/
VOID RsvSta_CheckIfRsvStationCanGetDataFromCDB(PBOOL pIsCPUreadyForHalt, PCONFIG pConfig);

#endif //RESERVATIONSTATIONS_H_

