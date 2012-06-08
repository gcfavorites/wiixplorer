#include "pdftool.h"

char *basename = nil;
pdf_xref *xref = nil;
int pagecount = 0;
static void (*cleanup)(void) = nil;

void closexref(void);

void die(fz_error error)
{
	fz_catch(error, "aborting");
	if (cleanup)
		cleanup();
	closexref();
}

void setcleanup(void (*func)(void))
{
	cleanup = func;
}

void openxref(char *filename, char *password, int dieonbadpass)
{
	fz_stream *file;
	int okay;
	int fd;

	basename = strrchr(filename, '/');
	if (!basename)
		basename = filename;
	else
		basename++;

	fd = open(filename, O_BINARY | O_RDONLY, 0666);
	if (fd < 0)
		die(fz_throw("cannot open file"));

	file = fz_openfile(fd);
	xref = pdf_openxref(file);
	if (!xref)
		die(-1);
	fz_dropstream(file);

	if (pdf_needspassword(xref))
	{
		okay = pdf_authenticatepassword(xref, password);
		if (!okay && !dieonbadpass)
			fz_warn("invalid password, attempting to continue.");
		else if (!okay && dieonbadpass)
			die(fz_throw("invalid password"));
	}

	pagecount = pdf_getpagecount(xref);
}

void flushxref(void)
{
	if (xref)
	{
		pdf_flushxref(xref, 0);
	}
}

void closexref(void)
{
	if (cleanup)
		cleanup();

	if (xref)
	{
		pdf_closexref(xref);
		xref = nil;
	}

	basename = nil;
}
