#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <dirent.h>

#define FILENAMELENGHT	70

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
