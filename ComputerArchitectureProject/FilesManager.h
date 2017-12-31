#ifndef FILEPARSER_H_
#define FILEPARSER_H_

#include "mainDefs.h"

typedef struct _CONFIG{
	UINT32 add_nr_units;
	UINT32 mul_nr_units;
	UINT32 div_nr_units;
	UINT32 add_nr_reservation;
	UINT32 mul_nr_reservation;
	UINT32 div_nr_reservation;
	UINT32 add_delay;
	UINT32 mul_delay;
	UINT32 div_delay;
	UINT32 mem_delay;
	UINT32 mem_nr_load_buffers;
	UINT32 mem_nr_store_buffers;
} CONFIG, *PCONFIG;

STATUS FilesManager_MeminParser(UINT32* memin, CPCHAR filename);

STATUS FilesManager_ConfigParser(PCONFIG pConfig, CPCHAR filename);

STATUS FilesManager_InitializeOutputFiles(CPCHAR memoutFile, CPCHAR regoutFile, CPCHAR traceinstFile, CPCHAR tracedbFile);

VOID FilesManager_FinalizeOutputFiles(VOID);

STATUS FilesManager_WriteRegisters(Register F[]);
STATUS FilesManager_WriteMemout(UINT32* mem);
STATUS FilesManager_WriteTraceinst(VOID);
STATUS FilesManager_WriteTracedb(pCDB CDBs, UINT32 CC);
VOID FilesManager_AddToIssueArray(PInstCtx pInst);
VOID FilesManager_FinalizeInstructions(VOID);
#endif //FILEPARSER_H_