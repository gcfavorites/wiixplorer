#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <dirent.h>

#include "fileops.h"

#define BLOCKSIZE               1048576      //1MB
#define FILENAMELENGHT	        70

char alldirfiles[300][FILENAMELENGHT];
char filenames[FILENAMELENGHT];

bool findfile(const char * filename, const char * path)
{
    DIR *dir;
    struct dirent *file;

    dir = opendir(path);

    char temp[11];
    while ((file = readdir(dir)))
    {
        snprintf(temp,sizeof(temp),"%s",file->d_name);
        if (!strncmpi(temp,filename,11))
            {
            //WindowPrompt(path, filename,"go" ,0);
            closedir(dir);
            return true;
            }
        }
      closedir(dir);
      return false;
}

bool checkfile(char * path)
{
    FILE * f;
    f = fopen(path,"rb");
    if(f) {
    fclose(f);
    return true;
    }
    fclose(f);
    return false;
}


int CopyFile(const char * src, const char * dest) {

	u32 blksize;
	size_t read = 1;

	FILE * source = fopen(src, "rb");

	if(!source){
		return -2;
	}

    fseek(source, 0, SEEK_END);
    u32 sizesrc = ftell(source);
    rewind(source);

    if(sizesrc < BLOCKSIZE)
        blksize = sizesrc;
    else
        blksize = BLOCKSIZE;

	unsigned char * buffer = (unsigned char*) malloc(blksize);

	if(buffer == NULL){
	    //no memory
        fclose(source);
		return -1;
	}

	FILE * destination = fopen(dest, "wb");

    if(destination == NULL) {
        free(buffer);
        fclose(source);
        return -3;
    }

    while (read > 0) {
        read = fread(buffer, 1, blksize, source);
        fwrite(buffer, 1, read, destination);
    }

    free(buffer);
    fclose(source);
    fclose(destination);

    //get size of written file
    destination = fopen(dest, "rb");
    fseek(destination , 0 , SEEK_END);
    u32 sizedest = ftell(destination);
    fclose(destination);

    if(sizesrc != sizedest) {
        return -4;
    }

	return 1;
}


bool subfoldercreate(char * fullpath) {
        //check forsubfolders
    char dircheck[300];
    char dirnoslash[300];
    char * pch = NULL;
    u32 cnt = 0;
    struct stat st;

    snprintf(dirnoslash, strlen(fullpath), "%s", fullpath);

    if(stat(fullpath, &st) != 0) {
        pch = strrchr(dirnoslash, '/');
        cnt = pch-dirnoslash;
        snprintf(dircheck, cnt+2, "%s", dirnoslash);
        subfoldercreate(dircheck);
    };

    if (mkdir(dirnoslash, 0777) == -1) {
        return false;
    }

    return true;
}

int filenamescmp(const void *a, const void *b)
{
	return stricmp((char *) a, (char *) b);
}

int GetAllDirFiles(char * filespath)
{
	int countfiles = 0;

	struct stat st;
	DIR_ITER* dir;
	dir = diropen (filespath);

	if (dir == NULL) //If empty
       return 0;
	while (dirnext(dir,filenames,&st) == 0)
	{
		if ((st.st_mode & S_IFDIR) == 0)
		{
			snprintf(alldirfiles[countfiles], FILENAMELENGHT, "%s", filenames);
			countfiles++;
		}
	}
	dirclose(dir);
	qsort(alldirfiles, countfiles, sizeof(char[FILENAMELENGHT]), filenamescmp);
	return countfiles;
}
