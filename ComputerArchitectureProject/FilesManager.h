#ifndef FILESMANAGER_H_
#define FILESMANAGER_H_

#include "mainDefs.h"



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
#endif //FILESMANAGER_H_