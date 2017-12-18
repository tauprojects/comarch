
#include <stdio.h>
#include <stdlib.h>


#define MEMORY_SIZE		4096
#define MAX_LINE_LEN	200
#define UINT32			unsigned int
#define MEMIN_FILENAME	"memin.txt"
#define CONF_FILENAME	"config.txt"
#define TRUE			1
#define FALSE			0

#define ASSIGN_IF(x)	if(!strcmp(#x,var))	{ pConfig->x = value; printf("\t%s = %d\n",#x,value); }

#define ASSIGN_ELIF(x)  else ASSIGN_IF(x)


typedef struct _CONFIG{
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


typedef enum _STATUS {
	STATUS_SUCCESS,
	STATUS_MEMORY_FAIL,
	STATUS_FILE_FAIL,
	STATUS_PARSE_FAIL,
	STATUS_STRTOL_FAIL,
	STATUS_WRONG_NAME_FAIL,
	STATUS_GENERAL_FAIL
} STATUS; 


STATUS FileParser_MeminParser(UINT32** pMemin)
{
	STATUS	status = STATUS_SUCCESS;
	UINT32*	memin = *pMemin;
	FILE*	meminFile = NULL;
	UINT32	memIndex = 0;
	char	line[200];
	UINT32	temp;
	char*	endPtr;
	int index;

	do {

		meminFile = fopen(MEMIN_FILENAME,"r");

		if(!meminFile)
		{
			status = STATUS_FILE_FAIL;
			break;
		}

		memin = calloc(MEMORY_SIZE*4,sizeof(UINT32));

		if(!memin)
		{
			status = STATUS_MEMORY_FAIL;
			fclose(meminFile);
			break;
		}

		while(!feof(meminFile) && memIndex < MEMORY_SIZE)
		{
			if(!fgets(line,MAX_LINE_LEN,meminFile))
			{
				status = STATUS_PARSE_FAIL;
				fclose(meminFile);
				free(memin);
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
				free(memin);
				break;
			}

			memin[memIndex++] = temp;
		}

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

			ret = sscanf(line,"%s = %d",var,&value);
			
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


int main(int argc, int** argv)
{

	UINT32* memin = NULL;
	STATUS status = STATUS_SUCCESS;
	int index;
	CONFIG config = { 0 };
	
	status = FileParser_MeminParser(&memin);
	printf("FileParser_MeminParser returned with status %d\n",status);

	if(status)
	{
		getch();
		return status;
	}

	printf("Parsing Config File\n");

	status = FileParser_ConfigParser(&config);
	printf("FileParser_ConfigParser returned with status %d\n",status);

	getch();
	free(memin);
	return 0;
}








