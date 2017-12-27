
#include <stdlib.h>
#include "safeMalloc.h"

int	globalMemoryCounter = 0;


void* safeMalloc(size_t size)
{
	void* ret = malloc(size);

	if (ret)
		globalMemoryCounter++;

	return ret;
}

void* safeCalloc(size_t count, size_t size)
{
	void* ret = calloc(count, size);

	if (ret)
		globalMemoryCounter++;

	return ret;
}

void _safeFree(void* ptr, const char* file, const char* func, int line)
{
	if (ptr)
	{
		free(ptr);
		globalMemoryCounter--;
	}
	else
	{
		printf("#### Safe Free encountered NULL ptr on:\n\tFile: %s\n\tFunction: %s\n\tLine: %d\n",
			file, func, line);
	}

}

