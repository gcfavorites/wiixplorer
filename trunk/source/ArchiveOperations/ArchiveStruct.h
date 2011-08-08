#ifndef ARCHIVESTRUCT_H_
#define ARCHIVESTRUCT_H_

#include <gctypes.h>

typedef struct
{
	char * filename; // full filename
	size_t length; // uncompressed file length in 64 bytes for sizes higher than 4GB
	size_t comp_length; // compressed file length in 64 bytes for sizes higher than 4GB
	bool isdir; // 0 - file, 1 - directory
	u32 fileindex; // fileindex number
	u64 ModTime; // modification time
	u8 archiveType; // modification time
} ArchiveFileStruct;

enum
{
	UNKNOWN = 1,
	ZIP,
	SZIP,
	RAR,
	U8Arch,
	ArcArch
};

#endif
