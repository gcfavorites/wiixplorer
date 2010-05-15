#include "fitz.h"
#include "Tools/tools.h"

void * fz_malloc(int n)
{
	void *p = malloc(n);
	if (!p)
	{
		ShowError("Fatal Error: out of memory. Exiting...");
		abort();
	}
	return p;
}

void * fz_realloc(void *p, int n)
{
	void *np = realloc(p, n);
	if (np == nil)
	{
		ShowError("Fatal Error: out of memory. Exiting...");
		abort();
	}
	return np;
}

void fz_free(void *p)
{
	free(p);
}

char * fz_strdup(char *s)
{
	char *ns = malloc(strlen(s) + 1);
	if (!ns)
	{
		ShowError("Fatal Error: out of memory. Exiting...");
		abort();
	}
	memcpy(ns, s, strlen(s) + 1);
	return ns;
}

