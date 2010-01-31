#include <gctypes.h>
#include <stdio.h>
#include <malloc.h>

extern __typeof(malloc) __real_malloc;
extern __typeof(calloc) __real_calloc;
extern __typeof(realloc) __real_realloc;
extern __typeof(memalign) __real_memalign;
extern __typeof(free) __real_free;
extern __typeof(malloc_usable_size) __real_malloc_usable_size;

#ifdef DEBUG_MEM
static int mallocCnt = 0;
static int callocCnt = 0;
static int memalignCnt = 0;
static int reallocCnt = 0;
static int malloc_usable_sizeCnt = 0;
static int freeCnt = 0;
static FILE * dbgFile = NULL;
static BOOL mem_debugging = false;
#endif


void *__wrap_malloc(size_t size)
{

    #ifdef DEBUG_MEM
    if(mem_debugging)
    {
        if(size > 0)
        {
            mallocCnt++;
            fprintf(dbgFile, "\nmalloc %i: %i", mallocCnt, size);
        }
    }
    #endif

	return __real_malloc(size);
}

void *__wrap_calloc(size_t n, size_t size)
{
    #ifdef DEBUG_MEM
    if(mem_debugging)
    {
        if(size > 0)
        {
            callocCnt++;
            fprintf(dbgFile, "\ncalloc %i: %i", callocCnt, size);
        }
    }
    #endif

    return __real_calloc(n, size);
}

void *__wrap_memalign(size_t a, size_t size)
{
    #ifdef DEBUG_MEM
    if(mem_debugging)
    {
        if(size > 0)
        {
            memalignCnt++;
            fprintf(dbgFile, "\nmemalign %i: %i", memalignCnt, size);
        }
    }
    #endif

	return __real_memalign(a, size);
}

void __wrap_free(void *p)
{
    if(!p)
        return;

    #ifdef DEBUG_MEM
    if(mem_debugging)
    {
        freeCnt++;
        fprintf(dbgFile, "\nfree %i", freeCnt);
    }
    #endif

	__real_free(p);
}

void *__wrap_realloc(void *p, size_t size)
{
    #ifdef DEBUG_MEM
    if(mem_debugging)
    {
        if(size > 0)
        {
            reallocCnt++;
            fprintf(dbgFile, "\nrealloc %i: %i", reallocCnt, size);
        }
    }
    #endif

	return __real_realloc(p, size);
}

size_t __wrap_malloc_usable_size(void *p)
{
    #ifdef DEBUG_MEM
    if(mem_debugging)
    {
        malloc_usable_sizeCnt++;
        fprintf(dbgFile, "\nmalloc_usable_size %i", malloc_usable_sizeCnt);
    }
    #endif

	return __real_malloc_usable_size(p);
}

#ifdef DEBUG_MEM
void StartMemDebug()
{
    if(dbgFile)
        return;

    dbgFile = fopen("sd:/meminfo.txt", "wb");
    if(!dbgFile)
    {
        fclose(dbgFile);
        dbgFile = NULL;
        return;
    }

    mallocCnt = 0;
    callocCnt = 0;
    memalignCnt = 0;
    reallocCnt = 0;
    malloc_usable_sizeCnt = 0;
    freeCnt = 0;
    mem_debugging = true;
}

void StopMemDebug()
{
    if(!mem_debugging)
        return;

    mem_debugging = false;
    fprintf(dbgFile, "\n\nTotal Counts:");
    fprintf(dbgFile, "\nAllocations: %i", mallocCnt+callocCnt+memalignCnt);
    fprintf(dbgFile, "\nFrees: %i", freeCnt);
    fprintf(dbgFile, "\nDifference: %i", mallocCnt+callocCnt+memalignCn-freeCnt);
    fprintf(dbgFile, "\n#End of File");

    fclose(dbgFile);
    dbgFile = NULL;
}

#endif //DEBUG_MEM
