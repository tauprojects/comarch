#ifndef FUNCTIONALUNITS_H_
#define FUNCTIONALUNITS_H_

#include "mainDefs.h"

extern UINT32			mem[];

extern pRsvStation		reservationStations[];
extern pRsvStation		AddRsvStations;
extern pRsvStation		MulRsvStations;
extern pRsvStation		DivRsvStations;
extern pRsvStation		LoadBuffers;
extern pRsvStation		StoreBuffers;

extern UINT32			CC;
extern _CDB				CDBs[];

VOID CPU_InitializeMemoryUnit(PCONFIG pConfig);
VOID CPU_ProcessFunctionalUnits(PBOOL pisCPUreadyForHalt);

VOID CPU_ProcessMemoryUnit(PBOOL pIsCPUreadyForHalt);
#endif FUNCTIONALUNITS_H_