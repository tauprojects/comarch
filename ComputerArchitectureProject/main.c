#include <stdio.h>
#include <stdlib.h>
#include "FileParser.h"

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