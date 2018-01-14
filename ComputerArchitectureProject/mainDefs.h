#ifndef MAINDEFS_H_
#define MAINDEFS_H_

#include "safeMalloc.h"

/**
 * This enables/disables log printing to the command console. Used for debug purposes.
 */

#define DEBUG 0
#if DEBUG == 1
#define dprintf(...)	printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

/************************************************************************/
/* Public Constants                                                     */
/************************************************************************/
#define MEMORY_SIZE				4096
#define MAX_LINE_LEN			200
#define TRUE					1
#define FALSE					0
#define NUM_REGS				16
#define NUM_CDBS				4
#define INSTRUCTION_QUEUE_LEN	16

/**
 * Those macros are used because the memory is defined as UINT32. Values read from the memory
 * are converted using this "trick" to floating point representation. They are also converted back to hex
 * so they can be simply written to the MEMOUT file.
 */
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

/**
 * Those typedefs are for cleanness of the code.
 */
typedef unsigned int	UINT32, *PUINT32;
typedef int				INT32,	*PINT32;
typedef void			VOID,	*PVOID;
typedef UINT32			BOOL,	*PBOOL;
typedef char			CHAR,	*PCHAR,	**PPCHAR;
typedef const char*		CPCHAR;
typedef float			FLOAT;

/**
 * This enum is defined mainly for debug purposes, enables return of menaingfull information
 * from core functions.
 */
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
	STATUS_INVALID_QUEUE
} STATUS; 

/**
* This struct is the configuration of the program. It is filled by the #FilesManager_ParseConfig function.
*/
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

/**
 * This enum is the OPCODE enum, and has the same values as the bits encoded in the instruction.
 */
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

/**
 * This struct represent a parsed instruction and all its properties.
 * The instruction is allocated dynamically and its pointer is passed between instruction queue,
 * reservation stations, functional units and CDB.
 * It finally resides on the "issue array", which is printed to the TraceInst file in the same order
 * as they were issued.
 * It is also filled with all the information needed for the printing.
 */
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

/**
 * This is used as a reference to how many opcodes there are - FUNC_CNT.
 */
typedef enum _eFunctionOp
{
	FUNC_LD = LD,
	FUNC_ST = ST,
	FUNC_ADD = ADD,
	FUNC_MULT = MULT,
	FUNC_DIV = DIV,
	FUNC_CNT
} eFunctionOp, *peFunctionOp;


/**
 * Struct holding a functional unit. This struct represents all types of functional units, including memory.
 */
typedef struct _FunctionUnit
{
	UINT32			numOfFunctionalUnitsFromThisType;	//used to know how many FU from this type comfortably, filled in init phase
	eFunctionOp		type;								//used to find corresponding RSV station from FU type
	BOOL			busy;								//indicates whether the FU is currently holding instruction
	UINT32			delay;								//holds the delay of the FU, filled in init phase
	CHAR			name[10];							//for debugging purposes
	PInstCtx		pInstruction;						//holds the instruction currently in the FU
	UINT32			clockCycleCounter;					//counter for the current FU execution, counts until delay

	FLOAT			SRC0;								//copied from the Vj in RSV station
	FLOAT			SRC1;								//copied from the Vk in RSV station
	FLOAT			DST;								//holds the result of the operation or the value needs to be stored.


} FunctionUnit, *pFunctionUnit;

/**
 * Struct holding a SINGLE reservation station. Holds all types of reservation stations,
 * including memory buffer.
 */
typedef struct _RsvStation
{
	UINT32			numOfRsvStationsFromThisType;	//used to know how many FU from this type comfortably, filled in init phase
	CHAR			name[10];						//holds the name of the station, "tag" name, printed in files afterwards.
	BOOL			busy;							//indicates if the station holds an instruction currently
	BOOL			isInstInFuncUnit;				//station continue to hold instruction even after they are passed
													//to the FU, this indicates whether it has passed already to the FU.
													//so the instruciton won't be passed to the FU more than once.
	
	PInstCtx		pInstruction;					//holds the instruction currently in the RSV STA.
	UINT32			Address;						//for store operations / memory buffer usage, used to indicate if
													//the address can be written to instantaneously or needs to wait 
													//for another memory instruction to finish beforehand.

	//For Reservation stations ADD,MULT,DIV
	FLOAT			Vj;
	FLOAT			Vk;
	pRsvStation		Qj;								//The tags are simply a pointer to the relevant reservation station,
	pRsvStation		Qk;								//They can be thus simply compared to know if they are equal.

	pFunctionUnit	functionalUnits;				//Relevant functional unit array from the same type
	
} RsvStation, *pRsvStation;

/**
 * A struct representing a register, the register has a value and a tag. 
 */
typedef struct _register 
{
	FLOAT			value;		//value in the register
	BOOL			hasTag;		//boolean indicating whether theres a tag currently

	pRsvStation		tag;		//Tag - pointer to the relevant reservation station.

	PInstCtx		inst;		//This holds the instruction that is currently being executed to calculate
								//the value inside the reservation station represented by "tag".
								//To differentiate tags in the case there is the same tags
								//from different instructions after one instruction was freed
								//from reservation station and the same reservation station
								//got a different instruction in the meantime

} Register, *pRegister;

/**
 * Struct holding the CDB.
 */
typedef struct __cdb
{
	pRsvStation tag;			//Tag - pointer to the relevant reservation station. 
								//Also indicates if the CDB is currently broadcasting - if not its NULL.
	
	PInstCtx	inst;			//Used to know details about the instruction that lead to the value

	FLOAT		value;			//value broadcasted
} _CDB, *pCDB;

#endif //MAINDEFS_H_