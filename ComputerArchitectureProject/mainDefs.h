#ifndef MAINDEFS_H_
#define MAINDEFS_H_

#include "safeMalloc.h"

#define breakpoint			printf("## BREAKPOINT: %s | %d\n",__FUNCTION__,__LINE__)

#define DEBUG 1

#if DEBUG == 1
#define dprintf(...)	printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

#define MEMORY_SIZE		4096
#define MAX_LINE_LEN	200
#define TRUE			1
#define FALSE			0
#define NUM_REGS		16
#define NUM_CDBS		4
#define INSTRUCTION_QUEUE_LEN	16

#define Hex2Float(x)			*((float*)&(x));	
#define Float2Hex(x)			*((unsigned int*)&(x));	

/**
* This macro runs a function with its argument, and then checks if the
* returned status is SUCCESS (status != 0).
* Requires an initialized STATUS status variable
*/
#define	runCheckStatusBreak(func,...)										\
{																			\
	status = func(__VA_ARGS__);												\
	if (status)																\
	{																		\
		printf("Function [%s] returned with status %d\n", #func, status);	\
		break;																\
	}																		\
}

typedef unsigned int	UINT32, *PUINT32;
typedef int				INT32,	*PINT32;
typedef void			VOID,	*PVOID;
typedef UINT32			BOOL,	*PBOOL;
typedef char			CHAR,	*PCHAR,	**PPCHAR;
typedef const char*		CPCHAR;

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

typedef struct _CONFIG {
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
	CHAR			name[10];
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
	BOOL			isInstInFuncUnit;
	PInstCtx		pInstruction;
	UINT32			Address;		//for store operations / memory buffer usage

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
	PInstCtx		inst;		//to differentiate tags in the case there is the same tags
								//from different instructions after one instruction was freed
								//from reservation station and the same reservation station
								//got a different instruction in the meantime

} Register, *pRegister;

typedef struct __cdb
{
	pRsvStation tag;
	PInstCtx	inst;

	float		value;
	UINT32		CCupdated;
} _CDB, *pCDB;

#endif //MAINDEFS_H_