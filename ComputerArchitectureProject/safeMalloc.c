
#include <stdlib.h>
#include "safeMalloc.h"

#define LEN	1000

int	globalMemoryCounter = 0;
int arrayCounter = 0;
void* pointers_array[LEN];

void* safeMalloc(size_t size)
{
	void* ret = malloc(size);

	if (ret) 
	{
		globalMemoryCounter++;
		if(arrayCounter < LEN)
		{
			pointers_array[arrayCounter++] = ret;
		}
	}

	return ret;
}

void* safeCalloc(size_t count, size_t size)
{
	void* ret = calloc(count, size);

	if (ret)
	{
		globalMemoryCounter++;
		if (arrayCounter < LEN)
		{
			pointers_array[arrayCounter++] = ret;
		}
	}

	return ret;
}

void _safeFree(void* ptr, void** pPtr, const char* file, const char* func, int line)
{
	if (ptr)
	{
		free(ptr);
		globalMemoryCounter--;

		for (int i = 0; i < arrayCounter; i++)
		{
			if (ptr == pointers_array[i]) 
			{
				pointers_array[i] = NULL;
				break;
			}
		}
	}
	else
	{
		printf("#### Safe Free encountered NULL ptr on:\n\tFile: %s\n\tFunction: %s\n\tLine: %d\n",
			file, func, line);
	}

	*pPtr = NULL;
}

void cleanMemory(void)
{
	for (int i = 0; i < arrayCounter; i++)
	{
		if (pointers_array[i])
		{
			free(pointers_array[i]);
			globalMemoryCounter--;
		}
	}
}