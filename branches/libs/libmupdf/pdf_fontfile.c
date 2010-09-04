#include "fitz.h"
#include "mupdf.h"

enum
{
	FD_FIXED = 1 << 0,
	FD_SERIF = 1 << 1,
	FD_SYMBOLIC = 1 << 2,
	FD_SCRIPT = 1 << 3,
	FD_NONSYMBOLIC = 1 << 5,
	FD_ITALIC = 1 << 6,
	FD_ALLCAP = 1 << 16,
	FD_SMALLCAP = 1 << 17,
	FD_FORCEBOLD = 1 << 18
};

enum { CNS, GB, Japan, Korea };
enum { MINCHO, GOTHIC };
static const char * configpath = NULL;

static const struct
{
	const char *name;
	const char *filename;
	} basefonts[] = {
	{ "Courier",
		"NimbusMonL-Regu.cff" },
	{ "Courier-Bold",
		"NimbusMonL-Bold.cff" },
	{ "Courier-Oblique",
		"NimbusMonL-ReguObli.cff" },
	{ "Courier-BoldOblique",
		"NimbusMonL-BoldObli.cff" },
	{ "Helvetica",
		"NimbusSanL-Regu.cff" },
	{ "Helvetica-Bold",
		"NimbusSanL-Bold.cff" },
	{ "Helvetica-Oblique",
		"NimbusSanL-ReguItal.cff" },
	{ "Helvetica-BoldOblique",
		"NimbusSanL-BoldItal.cff" },
	{ "Times-Roman",
		"NimbusRomNo9L-Regu.cff" },
	{ "Times-Bold",
		"NimbusRomNo9L-Medi.cff" },
	{ "Times-Italic",
		"NimbusRomNo9L-ReguItal.cff" },
	{ "Times-BoldItalic",
		"NimbusRomNo9L-MediItal.cff" },
	{ "Symbol",
		"StandardSymL.cff" },
	{ "ZapfDingbats",
		"Dingbats.cff" },
	{ "Chancery",
		"URWChanceryL-MediItal.cff" },
	{ nil, nil }
};

static const unsigned char * fallbackfont = NULL;
static int fallbackfont_size = 0;

void SetupPDFFallbackFont(const unsigned char * font, int size)
{
    fallbackfont = font;
    fallbackfont_size = size;
}

void SetupPDFFontPath(const char * path)
{
    configpath = path;
}

fz_error
pdf_loadbuiltinfont(pdf_fontdesc *fontdesc, char *fontname)
{
	fz_error error;
	int i;

	for (i = 0; basefonts[i].name; i++)
		if (!strcmp(fontname, basefonts[i].name))
			goto found;

	return fz_throw("cannot find font: '%s'. Put the .cff file into %s/fonts/", fontname, configpath);

found:
	pdf_logfont("load builtin font %s\n", fontname);

	char filepath[200];
	snprintf(filepath, sizeof(filepath), "%s/fonts/%s", configpath, basefonts[i].filename);

	error = fz_newfontfromfile(&fontdesc->font, filepath, 0);
	if (error)
		return fz_rethrow(error, "cannot load freetype font from buffer");

	fz_strlcpy(fontdesc->font->name, fontname, sizeof fontdesc->font->name);

	return fz_okay;
}

static fz_error
loadsystemcidfont(pdf_fontdesc *fontdesc, int ros, int kind)
{
#ifndef NOCJK
	fz_error error;
	/* We only have one builtin fallback font, we'd really like
	 * to have one for each combination of ROS and Kind.
	 */
	pdf_logfont("loading builtin CJK font\n");
	error = fz_newfontfrombuffer(&fontdesc->font,
		(unsigned char *) fallbackfont,
		fallbackfont_size, 0);
	if (error)
		return fz_rethrow(error, "cannot load builtin CJK font");
	fontdesc->font->ftsubstitute = 1; /* substitute font */
	return fz_okay;
#else
	return fz_throw("no builtin CJK font file");
#endif
}

fz_error
pdf_loadsystemfont(pdf_fontdesc *fontdesc, char *fontname, char *collection)
{
	fz_error error;
	char *name;

	int isbold = 0;
	int isitalic = 0;
	int isserif = 0;
	int isscript = 0;
	int isfixed = 0;

	if (strstr(fontname, "Bold"))
		isbold = 1;
	if (strstr(fontname, "Italic"))
		isitalic = 1;
	if (strstr(fontname, "Oblique"))
		isitalic = 1;

	if (fontdesc->flags & FD_FIXED)
		isfixed = 1;
	if (fontdesc->flags & FD_SERIF)
		isserif = 1;
	if (fontdesc->flags & FD_ITALIC)
		isitalic = 1;
	if (fontdesc->flags & FD_SCRIPT)
		isscript = 1;
	if (fontdesc->flags & FD_FORCEBOLD)
		isbold = 1;

	pdf_logfont("fixed-%d serif-%d italic-%d script-%d bold-%d\n",
		isfixed, isserif, isitalic, isscript, isbold);

	if (collection)
	{
		int kind;

		if (isserif)
			kind = MINCHO;
		else
			kind = GOTHIC;

		if (!strcmp(collection, "Adobe-CNS1"))
			return loadsystemcidfont(fontdesc, CNS, kind);
		else if (!strcmp(collection, "Adobe-GB1"))
			return loadsystemcidfont(fontdesc, GB, kind);
		else if (!strcmp(collection, "Adobe-Japan1"))
			return loadsystemcidfont(fontdesc, Japan, kind);
		else if (!strcmp(collection, "Adobe-Japan2"))
			return loadsystemcidfont(fontdesc, Japan, kind);
		else if (!strcmp(collection, "Adobe-Korea1"))
			return loadsystemcidfont(fontdesc, Korea, kind);

		fz_warn("unknown cid collection: %s", collection);
	}

	if (isscript)
		name = "Chancery";

	else if (isfixed)
	{
		if (isitalic) {
			if (isbold) name = "Courier-BoldOblique";
			else name = "Courier-Oblique";
		}
		else {
			if (isbold) name = "Courier-Bold";
			else name = "Courier";
		}
	}

	else if (isserif)
	{
		if (isitalic) {
			if (isbold) name = "Times-BoldItalic";
			else name = "Times-Italic";
		}
		else {
			if (isbold) name = "Times-Bold";
			else name = "Times-Roman";
		}
	}

	else
	{
		if (isitalic) {
			if (isbold) name = "Helvetica-BoldOblique";
			else name = "Helvetica-Oblique";
		}
		else {
			if (isbold) name = "Helvetica-Bold";
			else name = "Helvetica";
		}
	}

	error = pdf_loadbuiltinfont(fontdesc, name);
	if (error)
		return fz_throw("cannot load builtin substitute font: %s", name);

	/* it's a substitute font: override the metrics */
	fontdesc->font->ftsubstitute = 1;

	return fz_okay;
}

fz_error
pdf_loadembeddedfont(pdf_fontdesc *fontdesc, pdf_xref *xref, fz_obj *stmref)
{
	fz_error error;
	fz_buffer *buf;

	pdf_logfont("load embedded font\n");

	error = pdf_loadstream(&buf, xref, fz_tonum(stmref), fz_togen(stmref));
	if (error)
		return fz_rethrow(error, "cannot load font stream");

	error = fz_newfontfrombuffer(&fontdesc->font, buf->rp, buf->wp - buf->rp, 0);
	if (error)
	{
		fz_dropbuffer(buf);
		return fz_rethrow(error, "cannot load embedded font (%d %d R)", fz_tonum(stmref), fz_togen(stmref));
	}

	fontdesc->buffer = buf->rp; /* save the buffer so we can free it later */
	fz_free(buf); /* only free the fz_buffer struct, not the contained data */

	fontdesc->isembedded = 1;

	return fz_okay;
}
