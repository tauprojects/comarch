#include <stdio.h>
#include <stdlib.h>
#include "FileParser.h"
#include "Queue.h"

int main(int argc, char** argv)
{

	UINT32* memin = NULL;
	STATUS status = STATUS_SUCCESS;
	int index;
	CONFIG config = { 0 };
	PQUEUE pQueue = NULL;
	status = FileParser_MeminParser(&memin);
	printf("FileParser_MeminParser returned with status %d\n",status);

	if(status)
	{
		return status;
	}

	printf("Parsing Config File\n");

	status = FileParser_ConfigParser(&config);
	printf("FileParser_ConfigParser returned with status %d\n",status);

	
	status = Queue_Create(&pQueue,5);
	printf("Queue_Create returned with status %d\n",status);
	Queue_Print(pQueue);
	printf("\n");

	for(index = 0; index<6;index++)
	{
		status = Queue_Enqueue(pQueue,index,index+1);
		printf("Queue_Create returned with status %d\n",status);
		printf("\n");
		Queue_Print(pQueue);
		printf("\n");
	}





	status = Queue_Destroy(pQueue);
	printf("Queue_Destroy returned with status %d\n",status);
	
	free(memin);
	
	return 0;
}