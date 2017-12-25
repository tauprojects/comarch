#ifndef MAINDEFS_H_
#define MAINDEFS_H_

#define MEMORY_SIZE		4096
#define MAX_LINE_LEN	200
#define TRUE			1
#define FALSE			0

typedef unsigned int	UINT32, *PUINT32;
typedef int				INT32,	*PINT32;
typedef void			VOID,	*PVOID;
typedef UINT32			BOOL,	*PBOOL;

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

typedef struct _InstCtx
{
	UINT32	inst;
	UINT32	pc;
	UINT32	opcode;
	UINT32	DST;
	UINT32	SRC0;
	UINT32	SRC1;
	UINT32	IMM;

	UINT32	tag;
	UINT32	cycleIssued;
	UINT32	cycleExecutionStart;
	UINT32	cycleExecutionEnd;
	INT32	cycleWriteCDB;
} InstCtx, *PInstCtx, **PPInstCtx;

#define MEMIN_FILENAME			"memin.txt"
#define CONF_FILENAME			"config.txt"
#define INSTRUCTION_QUEUE_LEN	16

#define Hex2Float(x)			*((float*)&x);

#endif //MAINDEFS_H_