#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

#include "mainDefs.h"

STATUS Instructions_ParseAndValidateCurrentPC(PInstCtx pInstCtx, UINT32 PC);

STATUS Instructions_FetchTwoInstructions(PQUEUE pInstQ, PBOOL pWasHalt, PUINT32 mem, PUINT32 pPC);

#endif  //INSTRUCTIONS_H_