
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "FilesManager.h"

#define FOPEN(f,n,t)		f = fopen(n,t);					\
							if (!f)				\
							{								\
								status = STATUS_FILE_FAIL;	\
								break;						\
							}

#define FCLOSE(f)			if (f) fclose(f);

#define ASSIGN_IF(x)	if(!strcmp(#x,var))	{ pConfig->x = value; printf("\t%s = %d\n",#x,value); }

#define ASSIGN_ELIF(x)  else ASSIGN_IF(x)

FILE* memout = NULL;
FILE* regout = NULL;
FILE* traceinst = NULL;
FILE* tracedb = NULL;

PInstCtx	instrctionByIssue[MEMORY_SIZE];
UINT32		issueCtr = 0;

CPCHAR		CDBnames[NUM_CDBS] = { "ADD", "MUL", "DIV", "MEM" };

static VOID FilesManager_GetLine(FILE* file, PCHAR line)
{
	char	ch;
	int		chInd = 0;

	while ((ch = fgetc(file)) != EOF)
	{
		line[chInd++] = ch;
		if (ch == '\n')
			break;
	}
	line[chInd - 1] = '\0';
}

STATUS FilesManager_MeminParser(UINT32* memin, CPCHAR filename)
{
	STATUS	status = STATUS_SUCCESS;
	FILE*	meminFile = NULL;
	UINT32	memIndex = 0;
	char	line[200];

	UINT32	temp;
	char*	endPtr;

	do {

		if(!memin)
		{
			status = STATUS_INVALID_ARGS;
			break;
		}
		
		meminFile = fopen(filename,"r");

		if(!meminFile)
		{
			printf("[fopen] failed with errno = %d", errno);
			status = STATUS_FILE_FAIL;
			break;
		}
		
		memset(memin, 0, MEMORY_SIZE * sizeof(UINT32));
		
		while(!feof(meminFile) && memIndex < MEMORY_SIZE)
		{
			FilesManager_GetLine(meminFile, line);

			if (line[0] && line[0] != '\n')
			{

				temp = (UINT32)strtol(line, &endPtr, 16);
				if (endPtr == line || *endPtr != '\0')
				{
					status = STATUS_STRTOL_FAIL;
					fclose(meminFile);
					break;
				}

				memin[memIndex] = temp;
			}
			memIndex++;
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

		configFile = fopen(filename,"r");

		if(!configFile)
		{
			status = STATUS_FILE_FAIL;
			break;
		}

		while(!feof(configFile))
		{
			FilesManager_GetLine(configFile, line);

			if(*line == '\n')
				continue;

			ret = sscanf(line,"%s = %u",var,&value);
			
			if(ret == EOF || ret != 2)
			{
				status = STATUS_PARSE_FAIL;
				fclose(configFile);
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
				fclose(configFile);
				break;
			}

		}

	fclose(configFile);
		
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
	UINT32	index;
	do
	{
		if (!traceinst)
		{
			status = STATUS_FILE_FAIL;
			break;
		}
		for (int i = 0; i < issueCtr; i++)
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