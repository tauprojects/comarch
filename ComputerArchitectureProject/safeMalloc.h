#ifndef SAFEMALLOC_H_
#define SAFEMALLOC_H_


#define safeFree(__ptr)	_safeFree(__ptr, &(__ptr), __FILE__, __FUNCTION__, __LINE__);

void* safeMalloc(unsigned int size);

void* safeCalloc(unsigned int count, unsigned int size);

void _safeFree(void* ptr, void** pPtr, const char* file, const char* func, int line);

void cleanMemory(void);

#endif //SAFEMALLOC_H_