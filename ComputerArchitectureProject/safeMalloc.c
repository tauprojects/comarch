
#include <stdlib.h>
#include "safeMalloc.h"

/**
* The purpose of this module is to prevent memory leaks and to catch "double free" operations.
* It manages the memory allocated dynamically in the other modules, and frees it in the end of the program.
* The module contains the #globalMemoryCounter which was used during debug to check whether all malloced
* pointers were freed during the run of the program. 
*/

int	globalMemoryCounter = 0;

/**
* The modules keeps track of the allocated memory in the execution. Each pointer allocated is saved
* in the #pointers_array which is assumed to be of constant length LEN (if there are more pointers,
* then they are not saved in the array). At the end of the program we call #cleanMemory function that
* simply passes on the array and frees all the non-freed elements.
*/

#define LEN	1000

int arrayCounter = 0;
void* pointers_array[LEN];

/**
 * This function replaces the standard #malloc function. It increments the #globalMemoryCounter
 * and adds the pointer to the #pointers_array.
 */
void* safeMalloc(unsigned int size)
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

/**
* This function replaces the standard #calloc function. It increments the #globalMemoryCounter
* and adds the pointer to the #pointers_array.
*/
void* safeCalloc(unsigned int count, unsigned int size)
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

/**
* This function replaces the standard #free function. For every free, it searches for the pointer
* in the #pointers_array and if it finds one it NULLs the pointer, then freeing the pointer.
* This thus prevents the #cleanMemory function to free the same pointer twice at the end of the program.
* The function is not used by itself, but by the corresponding macro #safeFree (see h file).
* The function also gets by the macro the ADDRESS of the pointer passed to it, and NULLs the value in the address.
* Because the function checks whether a NULL pointer was passed, it prevents double free.
*/
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

/**
 * This function checks the #pointers_array up to the point it was filled (#arrayCounter),
 * and frees every pointer which is not NULL.
 *
 * This function simply REMOVES the need to free pointers during the execution of the program,
 * preventing all memory leaks that could be caused by malloc/calloc. 
 *
 * Note: this function does not prevents memory leaks by c library function allocating - such as #fopen
 * not used afterwards by #fclose.
 */
void cleanMemory(void)
{
	int j = 0;

	for (int i = 0; i < arrayCounter; i++)
	{
		if (pointers_array[i])
		{
			j++;
			free(pointers_array[i]);
			globalMemoryCounter--;
		}
	}

}