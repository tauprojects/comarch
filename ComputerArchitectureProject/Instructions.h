#ifndef INSTRUCTIONS_H_
#define INSTRUCTIONS_H_

#include "mainDefs.h"

typedef enum _InstType
{
	LD,
	ST,
	ADD,
	SUB,
	MULT,
	DIV,
	HALT,
	TYPE_CNT
} eInstType, *peInstType;


STATUS Instructions_ParseAndValidateCurrentPC(PInstCtx pInstCtx, UINT32 PC);

STATUS Instructions_FetchTwoInstructions(PQUEUE pInstQ, PUINT32 mem, PUINT32 pPC);



#endif  //INSTRUCTIONS_H_