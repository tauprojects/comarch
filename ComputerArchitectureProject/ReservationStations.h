#ifndef RESERVATIONSTATIONS_H_
#define RESERVATIONSTATIONS_H_

#include "Queue.h"

extern pRsvStation		reservationStations[];
extern pRsvStation		AddRsvStations;
extern pRsvStation		MulRsvStations;
extern pRsvStation		DivRsvStations;

extern pRsvStation		LoadBuffers;
extern pRsvStation		StoreBuffers;

extern _CDB				CDBs[];
extern Register			F[];

VOID RsvSta_InitializeReservationsStations(PCONFIG pConfig);
VOID RsvSta_IssueInstToRsvStations(PCONFIG pConfig, PBOOL pWasHolt, PQUEUE pInstQ, UINT32 CC);
VOID RsvSta_CheckIfRsvStationCanGetDataFromCDB(PBOOL pIsCPUreadyForHalt, PCONFIG pConfig);

#endif //RESERVATIONSTATIONS_H_

