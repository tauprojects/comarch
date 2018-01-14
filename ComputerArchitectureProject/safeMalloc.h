#ifndef SAFEMALLOC_H_
#define SAFEMALLOC_H_

/**
 * The purpose of this module is to prevent memory leaks and to catch "double free" operations.
 * It manages the memory allocated dynamically in the other modules, and frees it in the end of the program.
 * 
 * MORE INFORMATION in the c file.
 */

void* safeMalloc(unsigned int size);

void* safeCalloc(unsigned int count, unsigned int size);



/**
 * See use of macro in #_safeFree in the c file
 */
#define safeFree(__ptr)	_safeFree(__ptr, &(__ptr), __FILE__, __FUNCTION__, __LINE__);



void _safeFree(void* ptr, void** pPtr, const char* file, const char* func, int line);

void cleanMemory(void);

#endif //SAFEMALLOC_H_