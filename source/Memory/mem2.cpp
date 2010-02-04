
#include "mem2.hpp"
#include "mem2alloc.hpp"

#include <malloc.h>
#include <string.h>

#define MEM2_PRIORITY_SIZE	2097152         //2MB

// Forbid the use of MEM2 through malloc
u32 MALLOC_MEM2 = 0;

static CMEM2Alloc g_mem2gp;

void MEM2_init(unsigned int mem2Size)
{
	g_mem2gp.init(mem2Size);
}

void MEM2_cleanup(void)
{
	g_mem2gp.cleanup();
}

extern "C" void *MEM2_alloc(unsigned int s)
{
	return g_mem2gp.allocate(s);
}

extern "C" void MEM2_free(void *p)
{
	g_mem2gp.release(p);
}

extern "C" void *MEM2_realloc(void *p, unsigned int s)
{
	return g_mem2gp.reallocate(p, s);
}

extern "C" unsigned int MEM2_usableSize(void *p)
{
	return CMEM2Alloc::usableSize(p);
}

bool g_bigGoesToMem2 = false;

void MEM2_takeBigOnes(bool b)
{
	g_bigGoesToMem2 = b;
}

extern "C"
{

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

	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		p = MEM2_alloc(size);
		if (p != 0) {
			return p;
		}
		return __real_malloc(size);
	}
	p = __real_malloc(size);
	if (p != 0) {
		return p;
	}
	return MEM2_alloc(size);
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

	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		p = MEM2_alloc(n * size);
		if (p != 0)
		{
			memset(p, 0, n * size);
			return p;
		}
		return __real_calloc(n, size);
	}
	p = __real_calloc(n, size);
	if (p != 0) {
		return p;
	}
	p = MEM2_alloc(n * size);
	if (p != 0) {
		memset(p, 0, n * size);
	}
	return p;
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

	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		if (a <= 32 && 32 % a == 0)
		{
			p = MEM2_alloc(size);
			if (p != 0) {
				return p;
			}
		}
		return __real_memalign(a, size);
	}
	p = __real_memalign(a, size);
	if (p != 0 || a > 32 || 32 % a != 0) {
		return p;
	}

	return MEM2_alloc(size);
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

	if (((u32)p & 0x10000000) != 0)
	{
		MEM2_free(p);
	}
	else
	{
		__real_free(p);
	}
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

	void *n;
	// ptr from mem2
	if (((u32)p & 0x10000000) != 0 || (p == 0 && g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE))
	{
		n = MEM2_realloc(p, size);
		if (n != 0) {
			return n;
		}
		n = __real_malloc(size);
		if (n == 0) {
			return 0;
		}
		if (p != 0)
		{
			memcpy(n, p, MEM2_usableSize(p) < size ? MEM2_usableSize(p) : size);
			MEM2_free(p);
		}
		return n;
	}
	// ptr from malloc
	n = __real_realloc(p, size);
	if (n != 0) {
		return n;
	}
	n = MEM2_alloc(size);
	if (n == 0) {
		return 0;
	}
	if (p != 0)
	{
		memcpy(n, p, __real_malloc_usable_size(p) < size ? __real_malloc_usable_size(p) : size);
		__real_free(p);
	}
	return n;
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

	if (((u32)p & 0x10000000) != 0)
		return MEM2_usableSize(p);
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

} ///extern "C"
