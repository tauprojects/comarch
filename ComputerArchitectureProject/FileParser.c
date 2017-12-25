
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FileParser.h"


#define ASSIGN_IF(x)	if(!strcmp(#x,var))	{ pConfig->x = value; /*printf("\t%s = %d\n",#x,value);*/ }

#define ASSIGN_ELIF(x)  else ASSIGN_IF(x)



STATUS FileParser_MeminParser(UINT32* memin)
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
		
		meminFile = fopen(MEMIN_FILENAME,"r");

		if(!meminFile)
		{
			status = STATUS_FILE_FAIL;
			break;
		}
		
		memset(memin, 0, MEMORY_SIZE * sizeof(UINT32));
		
		while(!feof(meminFile) && memIndex < MEMORY_SIZE)
		{
			if(!fgets(line,MAX_LINE_LEN,meminFile))
			{
				status = STATUS_PARSE_FAIL;
				fclose(meminFile);
				break;
			}

			if(*line == '\n')
				continue;

			line[strlen(line)-1] = '\0';
			
			temp = (UINT32)strtol(line,&endPtr,16);
			if(endPtr == line || *endPtr != '\0')
			{
				status = STATUS_STRTOL_FAIL;
				fclose(meminFile);
				break;
			}

			memin[memIndex++] = temp;
		}
		
		fclose(meminFile);
		
	} while(FALSE);

	return status;
}


STATUS FileParser_ConfigParser(PCONFIG pConfig)
{
	STATUS	status = STATUS_SUCCESS;
	FILE*	configFile = NULL;
	char	line[200] = { 0 };
	char	var[30] = { 0 };
	UINT32	value;
	int		ret;

	do {

		configFile = fopen(CONF_FILENAME,"r");

		if(!configFile)
		{
			status = STATUS_FILE_FAIL;
			break;
		}

		while(!feof(configFile))
		{
			if(!fgets(line,MAX_LINE_LEN,configFile))
			{
				status = STATUS_PARSE_FAIL;
				fclose(configFile);	
				break;
			}

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











