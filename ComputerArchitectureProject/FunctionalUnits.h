#ifndef FUNCTIONALUNITS_H_
#define FUNCTIONALUNITS_H_

#include "mainDefs.h"

extern pRsvStation		reservationStations[];
extern pRsvStation		AddRsvStations;
extern pRsvStation		MulRsvStations;
extern pRsvStation		DivRsvStations;

extern UINT32			CC;
extern _CDB				CDBs[];

VOID CPU_ProcessFunctionalUnits(PBOOL pisCPUreadyForHalt);

#endif FUNCTIONALUNITS_H_