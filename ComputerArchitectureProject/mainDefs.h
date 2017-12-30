#ifndef MAINDEFS_H_
#define MAINDEFS_H_

#include "safeMalloc.h"

#define MEMORY_SIZE		4096
#define MAX_LINE_LEN	200
#define TRUE			1
#define FALSE			0
#define NUM_REGS		16
#define MEMIN_FILENAME			"memin.txt"
#define CONF_FILENAME			"config.txt"
#define INSTRUCTION_QUEUE_LEN	16

#define Hex2Float(x)			*((float*)&x);	
								

typedef unsigned int	UINT32, *PUINT32;
typedef int				INT32,	*PINT32;
typedef void			VOID,	*PVOID;
typedef UINT32			BOOL,	*PBOOL;
typedef char			CHAR,	*PCHAR,	**PPCHAR;

typedef enum _STATUS {
	STATUS_SUCCESS,
	STATUS_MEMORY_FAIL,
	STATUS_FILE_FAIL,
	STATUS_PARSE_FAIL,
	STATUS_STRTOL_FAIL,
	STATUS_WRONG_NAME_FAIL,
	STATUS_GENERAL_FAIL,
	STATUS_INVALID_ARGS,
	STATUS_QUEUE_FULL,
	STATUS_QUEUE_EMPTY,
	STATUS_INVALID_QUEUE,
	STATUS_INVALID_INSTRUCTION
} STATUS; 

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

typedef struct _RsvStation		RsvStation, *pRsvStation;

typedef struct _InstCtx
{
	UINT32	inst;
	UINT32	pc;
	UINT32	opcode;
	UINT32	DST;
	UINT32	SRC0;
	UINT32	SRC1;
	UINT32	IMM;

	pRsvStation	tag;

	UINT32		cycleIssued;
	UINT32		cycleExecutionStart;
	UINT32		cycleExecutionEnd;
	INT32		cycleWriteCDB;
} InstCtx, *PInstCtx, **PPInstCtx;

typedef enum _eFunctionOp
{
	FUNC_LD = LD,
	FUNC_ST = ST,
	FUNC_ADD = ADD,
	FUNC_MULT = MULT,
	FUNC_DIV = DIV,
	FUNC_CNT
} eFunctionOp, *peFunctionOp;

typedef struct _FunctionUnit
{
	UINT32			numOfFunctionalUnitsFromThisType;
	eFunctionOp		type;
	BOOL			busy;
	UINT32			delay;

	PInstCtx		pInstruction;
	UINT32			clockCycleCounter;

	float			SRC0;
	float			SRC1;
	float			DST;


} FunctionUnit, *pFunctionUnit;

typedef struct _RsvStation
{
	UINT32			numOfRsvStationsFromThisType;
	eFunctionOp		type;
	CHAR			name[10];
	BOOL			busy;
	PInstCtx		pInstruction;
	float			Address;		//for store operations / memory buffer usage

	//For Reservation stations ADD,MULT,DIV
	float			Vj;
	float			Vk;
	struct _RsvStation*		Qj;
	struct _RsvStation*		Qk;

	//Relevant functional unit array

	pFunctionUnit	functionalUnits;
	
} RsvStation, *pRsvStation;

typedef struct _register 
{
	float			value;
	BOOL			hasTag;

	pRsvStation		tag;
	UINT32			tagPC;		//to differentiate tags in the case there is the same tags
								//from different instructions after one instruction was freed
								//from reservation station and the same reservation station
								//got a different instruction in the meantime

} Register, *pResgister;

typedef struct __cdb
{
	pRsvStation tag;
	UINT32		tagPC;

	float		value;
	UINT32		CCupdated;
} _CDB, *pCDB;

#endif //MAINDEFS_H_