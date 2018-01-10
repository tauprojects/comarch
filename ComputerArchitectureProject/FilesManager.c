
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "FilesManager.h"

/************************************************************************/
/*				MACROS                                                  */
/************************************************************************/

#define FOPEN(f,n,t)		f = fopen(n,t);					\
							if (!f)				\
							{								\
								status = STATUS_FILE_FAIL;	\
								break;						\
							}

#define FCLOSE(f)			if (f) fclose(f);

#define ASSIGN_IF(x)	if(!strcmp(#x,var))	{ pConfig->x = value; printf("\t%s = %d\n",#x,value); }

#define ASSIGN_ELIF(x)  else ASSIGN_IF(x)

/************************************************************************/
/*				GLAOBALS                                                */
/************************************************************************/

FILE*		memout = NULL;
FILE*		regout = NULL;
FILE*		traceinst = NULL;
FILE*		tracedb = NULL;

PInstCtx	instrctionByIssue[MEMORY_SIZE];
UINT32		issueCtr = 0;

CPCHAR		CDBnames[NUM_CDBS] = { "ADD", "MUL", "DIV", "MEM" };

static BOOL FilesManager_GetLine(FILE* file, PCHAR line)
{
	int		ch;
	int		chInd = 0;
	BOOL	isEndOfFile = FALSE;

	while ((ch = fgetc(file)) != EOF)
	{
		line[chInd++] = (char)ch;
		if (ch == '\n' || ch == '\0' || ch == EOF)
			break;
	}

	if (ch != EOF && chInd > 0 && line[chInd - 1] == '\n')
		line[chInd - 1] = '\0';
	else if (ch == EOF)
		isEndOfFile = TRUE;

	return isEndOfFile;
}

STATUS FilesManager_MeminParser(UINT32* memin, CPCHAR filename)
{
	STATUS	status = STATUS_SUCCESS;
	FILE*	meminFile = NULL;
	UINT32	memIndex = 0;
	char	line[30];

	UINT32	temp;
	char*	endPtr;

	do {

		if(!memin)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}
		
		FOPEN(meminFile, filename, "r");
		
		memset(memin, 0, MEMORY_SIZE * sizeof(UINT32));
		
		while(!feof(meminFile) && memIndex < MEMORY_SIZE)
		{
			if (FilesManager_GetLine(meminFile, line))
			{
				NULL;
			}

			if (line[0] && line[0] != '\n')
			{

				temp = (UINT32)strtoul(line, &endPtr, 16);
				if (endPtr == line || *endPtr != '\0')
				{
					status = STATUS_STRTOL_FAIL;
					fclose(meminFile);
					break;
				}
				dprintf("mem[%d] = %x | line = %s\n", memIndex,temp,line);
				memin[memIndex] = temp;
			}
			memIndex++;
			memset(line, 0, 30);
		}
		
		fclose(meminFile);
		
	} while(FALSE);

	return status;
}


STATUS FilesManager_ConfigParser(PCONFIG pConfig, CPCHAR filename)
{
	STATUS	status = STATUS_SUCCESS;
	FILE*	configFile = NULL;
	char	line[200] = { 0 };
	char	var[30] = { 0 };
	UINT32	value;
	int		ret;

	do {

		FOPEN(configFile, filename, "r");

		while(!feof(configFile))
		{
			FilesManager_GetLine(configFile, line);

			if(*line == '\n')
				continue;

			ret = sscanf(line,"%s = %u",var,&value);
			
			if(ret == EOF || ret != 2)
			{
				status = STATUS_PARSE_FAIL;
				FCLOSE(configFile);
				break;
			}

			ASSIGN_IF(add_nr_units)
			ASSIGN_ELIF(mul_nr_units)
			ASSIGN_ELIF(div_nr_units)
			ASSIGN_ELIF(add_nr_reservation)
			ASSIGN_ELIF(mul_nr_reservation)
			ASSIGN_ELIF(div_nr_reservation)
			ASSIGN_ELIF(add_delay)
			ASSIGN_ELIF(mul_delay)
			ASSIGN_ELIF(div_delay)
			ASSIGN_ELIF(mem_delay)
			ASSIGN_ELIF(mem_nr_load_buffers)
			ASSIGN_ELIF(mem_nr_store_buffers)
			else
			{
				status = STATUS_WRONG_NAME_FAIL;
				FCLOSE(configFile);
				break;
			}

		}

	FCLOSE(configFile);
		
	} while(FALSE);

	return status;
}

STATUS FilesManager_InitializeOutputFiles(CPCHAR memoutFile, CPCHAR regoutFile, CPCHAR traceinstFile, CPCHAR tracedbFile)
{
	STATUS	status = STATUS_SUCCESS;

	do {

		FOPEN(memout, memoutFile, "w");
		FOPEN(regout, regoutFile, "w");
		FOPEN(traceinst, traceinstFile, "w");
		FOPEN(tracedb, tracedbFile, "w");
		
	} while (FALSE);

	return status;
}

VOID FilesManager_FinalizeOutputFiles(VOID)
{
	FCLOSE(memout);
	FCLOSE(regout);
	FCLOSE(tracedb);
	FCLOSE(traceinst);
}

STATUS FilesManager_WriteRegisters(Register F[])
{
	STATUS	status = STATUS_SUCCESS;
	UINT32	index;
	do 
	{
		if (!F)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}

		if (!regout)
		{
			status = STATUS_FILE_FAIL;
			break;
		}

		for (index = 0; index < NUM_REGS; index++)
		{
			//don't add newline for last line
			fprintf(regout, "%f\n", F[index].value);
		}

	} while (FALSE);

	return status;
}

STATUS FilesManager_WriteMemout(UINT32* mem)
{
	STATUS	status = STATUS_SUCCESS;
	UINT32	index;
	do
	{
		if (!mem)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}

		if (!memout)
		{
			status = STATUS_FILE_FAIL;
			break;
		}

		for (index = 0; index < MEMORY_SIZE; index++)
		{
			fprintf(memout, "%08x\n",mem[index]);
		}

	} while (FALSE);

	return status;
}

STATUS FilesManager_WriteTraceinst(VOID)
{
	STATUS	status = STATUS_SUCCESS;
	do
	{
		if (!traceinst)
		{
			status = STATUS_FILE_FAIL;
			break;
		}
		for (UINT32 i = 0; i < issueCtr; i++)
		{
			fprintf(traceinst, "%08x %d %s %d %d %d %d\n",
				instrctionByIssue[i]->inst,
				instrctionByIssue[i]->pc,
				instrctionByIssue[i]->tag->name,
				instrctionByIssue[i]->cycleIssued,
				instrctionByIssue[i]->cycleExecutionStart,
				instrctionByIssue[i]->cycleExecutionEnd,
				instrctionByIssue[i]->cycleWriteCDB);
		}

	} while (FALSE);

	return status;
}


VOID FilesManager_AddToIssueArray(PInstCtx pInst)
{
	instrctionByIssue[issueCtr++] = pInst;
}

STATUS FilesManager_WriteTracedb(pCDB CDBs, UINT32 CC)
{
	STATUS	status = STATUS_SUCCESS;
	UINT32	index;
	do
	{
		if (!tracedb)
		{
			status = STATUS_FILE_FAIL;
			break;
		}

		for (index = 0; index < NUM_CDBS; index++)
		{
			if (CDBs[index].tag != NULL)
			{
				dprintf("\nCDB <%s> has PC <%d> value <%.1f> with tag <%s>\n", CDBnames[index], CDBs[index].inst->pc, CDBs[index].value, CDBs[index].inst->tag->name);
				fprintf(tracedb, "%d %d %s %f %s\n", CC, CDBs[index].inst->pc, CDBnames[index], CDBs[index].value, CDBs[index].inst->tag->name);
			}
		}

	} while (FALSE);

	return status;
}