#include "fitz.h"
#include "Language/gettext.h"
#include "Tools/tools.h"
#include "sys.h"

void * fz_malloc(int n)
{
	void *p = malloc(n);
	if (!p)
	{
	    ShowError(tr("Fatal Error: Out of memory. Requested %0.2fKB memory. Must shutdown app."), n/1024.0f);
		Sys_BackToLoader();
	}
	return p;
}

void * fz_realloc(void *p, int n)
{
	void *np = realloc(p, n);
	if (np == nil)
	{
        ShowError(tr("Fatal Error: Out of memory. Requested %0.2fKB memory. Must shutdown app."), n/1024.0f);
		Sys_BackToLoader();
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
		ShowError(tr("Fatal Error: Out of memory. Requested %0.2fKB memory. Must shutdown app."), (strlen(s) + 1)/1024.0f);
		Sys_BackToLoader();
	}
	memcpy(ns, s, strlen(s) + 1);
	return ns;
}

