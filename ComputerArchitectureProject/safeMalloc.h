#ifndef SAFEMALLOC_H_
#define SAFEMALLOC_H_


#define safeFree(__ptr)	_safeFree(__ptr, __FILE__, __FUNCTION__, __LINE__);

void* safeMalloc(size_t size);

void* safeCalloc(size_t count, size_t size);

void _safeFree(void* ptr, const char* file, const char* func, int line);

#endif //SAFEMALLOC_H_