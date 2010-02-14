

/****************************************************************************
* Copyright (C) 2009
* by LilouMaster based on Copyright (c) 2004 Thomas Oswald
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any
* damages arising from the use of this software.
*
* Permission is granted to anyone to use this software for any
* purpose, including commercial applications, and to alter it and
* redistribute it freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you
* must not claim that you wrote the original software. If you use
* this software in a product, an acknowledgment in the product
* documentation would be appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and
* must not be misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
* distribution.
*
*
* for WiiXplorer 2009
***************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <sys/fcntl.h>

#include "FTPClientconfig.h"
#include "FTPClientDefinements.h"
#include "FTPClient.h"




using namespace nsFTP;

nsFTP::CFTPClient ftpClient;

#define _P(a) (&((const tstring)a))

const char *drive[13]= {"sd:/", "usb:/", "ntfs0:/", "ntfs1:/", "ntfs2:/",
"ntfs3:/", "ntfs4:/", "smb1:/", "smb2:/", "smb3:/", "smb4:/", "isfs:/", "nand:/"};

const tstring ftpstr ("ftp://");



bool slipfromright(const tstring *strn, char c, tstring *left, tstring *right)
{
    LPCTSTR   m_pText = strn->c_str();
    if(!m_pText) return false;
    int len = strn->size();
    char * lpsz = strrchr(m_pText , c);
    
    if (lpsz == NULL) {
        *left = *strn;
        *right = tstring ("");
        return false;
    }
    else if ((int)(lpsz - m_pText)+1 == len)  {
        *right= tstring ("");
        *left = strn->substr(0,(int)(len-1));
    }
    else {
        *left = strn->substr(0,(int)(lpsz - m_pText));
        *right = strn->substr((int)(lpsz - m_pText)+1,len-(int)(lpsz - m_pText)-1);
    }
    return true;
}



boolean slipfromleft(const tstring *strn, char c, tstring *left, tstring *right)
{
    LPCTSTR   m_pText = strn->c_str();
    if(!m_pText) return false;
    int len = strn->size();
    char * lpsz = strchr(m_pText , c);
    
    if (lpsz == NULL) {
        *left = *strn;
        *right = tstring ("");
        return false;
    }
    else if ((int)(lpsz - m_pText)+1 == len)  {
        *right= tstring ("");
        *left = strn->substr(0,(int)(len-1));
    }
    else {
        *left = strn->substr(0,(int)(lpsz - m_pText));
        *right = strn->substr((int)(lpsz - m_pText)+1,len-(int)(lpsz - m_pText)-1);
    }
    return true;
}



tstring extractpath(const tstring *pathname)
{
    tstring path;
    tstring name;
    
    slipfromright(pathname, '/', &path, &name);
    if (path == "" ) return tstring("/");
    return path;
}

tstring extractname(const tstring *pathname)
{
    tstring path;
    tstring name;
    
    slipfromright(pathname, '/', &path, &name);
    if (name=="") {
        return path;
    }
    return name;
}


const tstring extractpathfrurl(const char *cUrl)
{
    tstring url;
    tstring userandpasswd;
    tstring hostandportandpath;
    tstring hostandpath;
    tstring portandpath;
    tstring path;
    tstring port;
    tstring host;
    tstring fullurl(cUrl);
    
    url = fullurl.substr(ftpstr.size());
    
    if (!slipfromright(&url, '@', &userandpasswd, &hostandportandpath)) {
        // no user and passwd
        hostandportandpath = url;
    }
    
    if (!slipfromleft(&hostandportandpath, ':', &host, &portandpath)){
        // no port
        hostandpath = hostandportandpath;
        if (!slipfromleft(&hostandpath, '/', &host, &path)) return tstring("/");
        return "/" + path ;
    }
    else
    {
        // port exist
        if (!slipfromleft(&portandpath, '/', &port, &path)) return tstring("/");
        return "/" + path ;
    }
}

const tstring extracthostfrurl(const char *cUrl)
{
    tstring url;
    tstring userandpasswd;
    tstring hostandportandpath;
    tstring hostandpath;
    tstring portandpath;
    tstring path;
    tstring port;
    tstring host;
    tstring fullurl(cUrl);
    
    url = fullurl.substr(ftpstr.size());
    
    if (!slipfromright(&url, '@', &userandpasswd, &hostandportandpath)) {
        // no user and passwd
        hostandportandpath = url;
    }
    
    if (!slipfromleft(&hostandportandpath, ':', &host, &portandpath)){
        // no port
        hostandpath = hostandportandpath;
        if (!slipfromleft(&hostandpath, '/', &host, &path)) return hostandpath;
        return host;
    }
    return host;
}

const tstring extractuserfrurl(const char *cUrl)
{
    tstring url;
    tstring userandpasswd;
    tstring hostandportandpath;
    tstring hostandpath;
    tstring portandpath;
    tstring path;
    tstring passwd;
    tstring user;
    tstring fullurl(cUrl);

    url = fullurl.substr(ftpstr.size());
    
    if (!slipfromright(&url, '@', &userandpasswd, &hostandportandpath)) {
        // no user and passwd
        return tstring("");
    }
    
    if (!slipfromleft(&userandpasswd, ':', &user, &passwd)) return userandpasswd;
    return user;
    
}


const tstring extractpasswfrurl(const char *cUrl)
{
    tstring url;
    tstring userandpasswd;
    tstring hostandportandpath;
    tstring hostandpath;
    tstring portandpath;
    tstring path;
    tstring passwd;
    tstring user;
    tstring fullurl(cUrl);
    boolean cr;
    
    url = fullurl.substr(ftpstr.size());
    
    if (!slipfromright(&url, '@', &userandpasswd, &hostandportandpath)) {
        // no user and passwd
        return tstring("");
    }
    
    if (!slipfromleft(&userandpasswd, ':', &user, &passwd)) return tstring("");
    return passwd;
    
}


const tstring extractportfrurl(const char *cUrl)
{
    tstring url;
    tstring userandpasswd;
    tstring hostandportandpath;
    tstring hostandpath;
    tstring portandpath;
    tstring path;
    tstring port;
    tstring host;
    tstring fullurl(cUrl);
    
    url = fullurl.substr(ftpstr.size());
    
    if (!slipfromright(&url, '@', &userandpasswd, &hostandportandpath)) {
        // no user and passwd
        hostandportandpath = url;
    }
    
    if (!slipfromleft(&hostandportandpath, ':', &host, &portandpath)){
        // no port
        return tstring("21");
    }
    else
    {
        // port exist
        if (!slipfromleft(&portandpath, '/', &port, &path)) return portandpath;
        return port ;
    }
}


bool decomposeurl(const char *curl, tstring * user, tstring *password, tstring *host, u32 *cport, tstring *path)
{
    tstring port;
    
    *user = extractuserfrurl(curl);
    *password = extractpasswfrurl(curl);
    *host = extracthostfrurl(curl);
	port = extractportfrurl(curl);
    *cport = atoi(port.c_str());
    *path = extractpathfrurl(curl);
return true;
}



bool isFtpPath(const char *name)
{
    if (strncmp(name, "ftp://",6)==0) return true;
    return false;
}

DIR_ITER ftpdir;
static int filenum=0;


DIR_ITER* sdiropen ( const char *cpath)
{
    int cr;
    
    if (isFtpPath(cpath))
    {
        tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
        tstring ftpserverpassword;
        u32 ftpserverport = 21;
        
       if (!decomposeurl(cpath, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return 0;
 
       filenum=0;
       
        const tstring path = extractpathfrurl(cpath);
        boolean cr1, cr2;
        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {
            cr1 = ftpClient.ChangeWorkingDirectory(&path);
            cr2 = ftpClient.List(_P(""), /* mode zip */ false, /* fPasv */ false);
            if ( cr1 == FTP_ERROR || cr2 == FTP_ERROR) return 0;
        }
        return (DIR_ITER* )2;
    }
	else
	{
        return diropen (cpath);
    }
}



int sdirreset (DIR_ITER *dirState)
{
    int cr;
	if (dirState==(DIR_ITER*)0) return -1; 
    if (dirState==(DIR_ITER*)1)
    {
        cr = ftpClient.ChangeWorkingDirectory(_P("/"));
        cr = ftpClient.List(_P(""), /* mode zip */ false, /* fPasv */ false);
        // ftpClient.Logout();
        }
	else
	{
        return dirreset (dirState);
    }
return -1;
}

int sdirnext (DIR_ITER *dirState, char *cfilename, struct stat *filestat)
{
   if (dirState==(DIR_ITER*)0) return -1;
 
    if (dirState==(DIR_ITER*)1)
    {
        CFTPFileStatus* FileStat;
        //sgIP_dbgprint("Number of files %d\n",ftpClient.NumberOfFiles()) ;
		
        if (filenum >= ftpClient.NumberOfFiles()) {
            filenum=0;
			*cfilename = 0;
			filestat->st_mode =  0;
			filestat->st_mtime = 0;
            return -1;
        }
        else {
            FileStat = ftpClient.GetFileStat(filenum);
       //sgIP_dbgprint("Nprint of files %s\n",FileStat->Name().c_str()) ;
            strcpy(cfilename, FileStat->Name().c_str());
            filestat->st_size = FileStat->Size();
            if (FileStat->IsDirectory())
            filestat->st_mode = S_IFDIR ;
            else
            filestat->st_mode = S_IFREG ;
            
			filestat->st_mtime = FileStat->MTime();
            filenum++;
             return 0;
        }
    }
	else
	{
        return dirnext (dirState, cfilename, filestat);
    }
}

int sdirclose (DIR_ITER *dirState)
{
	if (dirState==(DIR_ITER*)0) return -1;
	
    if (dirState==(DIR_ITER*)1)
    {
        ftpClient.Logout();
        return 0;
        }else{
        return dirclose (dirState);
    }
}


int sremove (const char *cname )
{
    if (isFtpPath(cname))
    {
        int cr ;
        tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
        tstring ftpserverpassword;
        u32 ftpserverport = 21;

        if (!decomposeurl(cname, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return -1;
        
        CFTPFileStatus* FileStat;

        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {
			const tstring name = extractpathfrurl(cname);
			sgIP_dbgprint("remove file start");
			cr = ftpClient.Delete(&name);
			sgIP_dbgprint("remove fileend");
        
			sgIP_dbgprint("remove dir start");
        
			cr = ftpClient.RemoveDirectory(&name);
        	ftpClient.Logout();
		}
		return -1;
        }else{
        return remove (cname);
    }
}

bool sCheckFile(const char * filepath)
{
    if (isFtpPath(filepath))
    {
        return true;
        }else{
        return sCheckFile (filepath);
    }
    
}

int sstat(const char *cfilepath, struct stat *filestat)
{
    if (isFtpPath(cfilepath))
    {
        
        int cr ;
        int fileOk = false;
        tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
        tstring ftpserverpassword;
        u32 ftpserverport = 21;
        
       if (!decomposeurl(cfilepath, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return 0;
        
        const tstring pathname =extractpathfrurl(cfilepath);
        const tstring path = extractpath(&pathname);
        const tstring name = extractname(&pathname);
        CFTPFileStatus* FileStat;

        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {
 
        cr = ftpClient.ChangeWorkingDirectory(&path);
        cr = ftpClient.List(&path, /* mode zip */ false, /* fPasv */ false);
        
        for (unsigned int i=0; i<ftpClient.NumberOfFiles(); i++)
        {
            FileStat = ftpClient.GetFileStat(i);
            if (FileStat->Name() == name) {
                FileStat = ftpClient.GetFileStat(i);
                filestat->st_size = FileStat->Size();
                if (FileStat->IsDirectory())
                filestat->st_mode = S_IFDIR ;
                else
                filestat->st_mode = S_IFREG ;
                filestat->st_mtime = FileStat->MTime();
                fileOk = true;
                break;
            }
        }
        
	ftpClient.Logout();
        if (fileOk) return 0;
        return -1;
	} else { return -1;
	}
        }else{
        return stat(cfilepath, filestat);
    }
    
}

extern int LoadFileToMem(const char *filepath, u8 **inbuffer, u64 *size);

int sLoadFileToMem(const char *cfilepath, u8 **inbuffer, u64 *size)
{
    if (isFtpPath(cfilepath))
    {
        *inbuffer =0;
        *size = 0;
        
        boolean cr;
        const tstring filepath =extractpathfrurl(cfilepath);
        tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
        tstring ftpserverpassword;
        u32 ftpserverport = 21;
        
       if (!decomposeurl(cfilepath, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return 0;
        
        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {
        
         cr = ftpClient.DownloadFile(&filepath, _P("/sd:/tmpfile"), /*zip*/ true );
        if (cr) {
            LoadFileToMem("/sd:/tmpfile", inbuffer, size);
            remove ("/sd:/tmpfile");
        }
        ftpClient.Logout();
	} else { return -1;
	}
          }else{
        return LoadFileToMem(cfilepath, inbuffer, size);
    }
}

extern int LoadFileToMemWithProgress(const char *progressText, const char *filepath, u8 **inbuffer, u64 *size);

int sLoadFileToMemWithProgress(const char *progressText, const char *cfilepath, u8 **inbuffer, u64 *size)
{
    if (isFtpPath(cfilepath))
    {
        }else{
        return LoadFileToMem(cfilepath, inbuffer, size);
    }
}


int smkdir(const char * cfullpath, int mode)
{
    if (isFtpPath((char *)cfullpath))
    {
        int cr;
        tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
        tstring ftpserverpassword;
        u32 ftpserverport = 21;
        
        if (!decomposeurl(cfullpath, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return 0;
        
        const tstring fullpath =extractpathfrurl(cfullpath);
        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {

        cr = ftpClient.MakeDirectory(&fullpath);
        ftpClient.Logout();
        return cr;
	} else { return -1;}
    }else{
        return mkdir(cfullpath, mode);
    }
}

extern int CopyFile(const char * src, const char * dest);

int sCopyFile(const char * csrc, const char * cdest)
{
    
    if (isFtpPath((char *)csrc) && isFtpPath((char *)cdest))
    {
        int cr;
        const tstring src =extractpathfrurl(csrc);
        const tstring dest =extractpathfrurl(cdest);
        tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
		tstring ftpserverpassword;
        u32 ftpserverport = 21;
        
        if (!decomposeurl(csrc, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return 0;
        
        const tstring fullpath =extractpathfrurl(csrc);
        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {
        cr = ftpClient.DownloadFile(&src, _P("/sd:/tmpfile"), /*zip*/ true );
        cr = ftpClient.UploadFile(_P("/sd:/tmpfile"), &dest, /* zip */ false, /* over*/ true);
        remove ("/tmpfile");
        ftpClient.Logout();
	} else { return -1;}
    } else if (isFtpPath((char *)csrc) && !isFtpPath((char *)cdest))
    {
        int cr;
        const tstring src =extractpathfrurl(csrc);
        tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
		tstring ftpserverpassword;
        u32 ftpserverport = 21;
        
        if (!decomposeurl(csrc, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return 0;
        
        const tstring fullpath =extractpathfrurl(csrc);
        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {
        cr = ftpClient.DownloadFile(&src, _P(cdest), /*zip*/ true,  /* over*/ true); // transfert du fichier du serv
        ftpClient.Logout();
	} else { return -1;}
    }
    else if (!isFtpPath((char *)csrc) && isFtpPath((char *)cdest))
    {
        int cr;
         tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
		tstring ftpserverpassword;
        u32 ftpserverport = 21;
        
        if (!decomposeurl(cdest, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return 0;
        
        const tstring fullpath =extractpathfrurl(csrc);
        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {
        cr = ftpClient.UploadFile(_P(csrc), &ftpserverpath, /* zip */ true, /* over*/ true); // transfert toto du client vers serveur
        ftpClient.Logout();
	} else { return -1;}
    }
    else if (!isFtpPath((char *)csrc) && !isFtpPath((char *)cdest))
    {
        return CopyFile(csrc,cdest);
    }
    return -1;
}

int srename(const char *csrcpath, const char *cdestdir)
{
    if (isFtpPath((char *)csrcpath))
    {
        int cr;
        tstring ftpserverpath;
        tstring ftpserverhost;
        tstring ftpserveruser;
        tstring ftpserverpassword;
        u32 ftpserverport = 21;
        
        if (!decomposeurl(csrcpath, &ftpserveruser, &ftpserverpassword, &ftpserverhost, &ftpserverport, & ftpserverpath))
        return 0;
        
        const tstring srcpath =extractpathfrurl(csrcpath);
        const tstring destdir =extractpathfrurl(cdestdir);
        const tstring fullpath =extractpathfrurl(csrcpath);
        nsFTP::CLogonInfo logonInfo(ftpserverhost, ftpserverport, ftpserveruser,  ftpserverpassword);
        if (ftpClient.Init() &&
        ftpClient.Login(logonInfo)) {
        cr = ftpClient.Rename(&srcpath, &destdir);
        ftpClient.Logout();
	} else { return -1;}
    }
    {
        return rename(csrcpath, cdestdir);
    }
}





extern void assert (int line, char *file, int nbr) ;



#define CHECK if (cr==FTP_OK) sgIP_dbgprint("ftpMain ******** SUCCESS ********  %d\n",__LINE__); else  {sgIP_dbgprint("******** FAILED **********  %d\n",__LINE__);sleep(20);}




void slipfromright1(void)
{
    const tstring strn("hello@world");
    tstring left;
    tstring right;
    u32 cr = FTP_NOTOK;
    
    slipfromright(&strn, '@', &left, &right) ;
    sgIP_dbgprint("slip '%s' '%s', '%s' \n",strn.c_str(),left.c_str(),right.c_str());
    if (left == "hello" && right == "world")  cr = FTP_OK;
    CHECK
}

void slipfromright2(void)
{
    const tstring strn("@world");
    tstring left;
    tstring right;
    u32 cr = FTP_NOTOK;
    
    slipfromright(&strn, '@', &left, &right) ;
    sgIP_dbgprint("slip '%s' '%s', '%s' \n",strn.c_str(),left.c_str(),right.c_str());
    if (left == "" && right == "world")  cr = FTP_OK;
    CHECK
}


void slipfromright3(void)
{
    const tstring strn("hello@");
    tstring left;
    tstring right;
    u32 cr = FTP_NOTOK;
    
    slipfromright(&strn, '@', &left, &right) ;
    sgIP_dbgprint("slip '%s' '%s', '%s' \n",strn.c_str(),left.c_str(),right.c_str());
    if (left == "hello" && right == "")  cr = FTP_OK;
    CHECK
}


void slipfromright4(void)
{
    const tstring strn("hello");
    tstring left;
    tstring right;
    u32 cr = FTP_NOTOK;
    
    slipfromright(&strn, '@', &left, &right) ;
    sgIP_dbgprint("slip '%s' '%s', '%s' \n",strn.c_str(),left.c_str(),right.c_str());
    if (left == "hello" && right == "")  cr = FTP_OK;
    CHECK
}



void slipfromleft1(void)
{
    const tstring strn("hello@world");
    tstring left;
    tstring right;
    u32 cr = FTP_NOTOK;
    
    slipfromleft(&strn, '@', &left, &right) ;
    sgIP_dbgprint("slip '%s' '%s', '%s' \n",strn.c_str(),left.c_str(),right.c_str());
    if (left == "hello" && right == "world")  cr = FTP_OK;
    CHECK
}

void slipfromleft2(void)
{
    const tstring strn("@world");
    tstring left;
    tstring right;
    u32 cr = FTP_NOTOK;
    
    slipfromleft(&strn, '@', &left, &right) ;
    sgIP_dbgprint("slip '%s' '%s', '%s' \n",strn.c_str(),left.c_str(),right.c_str());
    if (left == "" && right == "world")  cr = FTP_OK;
    CHECK
}


void slipfromleft3(void)
{
    const tstring strn("hello@");
    tstring left;
    tstring right;
    u32 cr = FTP_NOTOK;
    
    slipfromleft(&strn, '@', &left, &right) ;
    sgIP_dbgprint("slip '%s' '%s', '%s' \n",strn.c_str(),left.c_str(),right.c_str());
    if (left == "hello" && right == "")  cr = FTP_OK;
    CHECK
}


void slipfromleft4(void)
{
    const tstring strn("hello");
    tstring left;
    tstring right;
    u32 cr = FTP_NOTOK;
    
    slipfromleft(&strn, '@', &left, &right) ;
    sgIP_dbgprint("slip '%s' '%s', '%s' \n",strn.c_str(),left.c_str(),right.c_str());
    if (left == "hello" && right == "")  cr = FTP_OK;
    CHECK
}



void extractpasswfrurl1(void)
{
    const char *unixp = "ftp://user:password@host:port/path";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl >%s< >%s< \n",unixp,path.c_str());
    if (path == "password")  cr = FTP_OK;
    CHECK
}


void extractpasswfrurl2(void)
{
const char *unixp = "ftp://alain:alain@192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "alain")  cr = FTP_OK;
    CHECK
}


void extractpasswfrurl3(void)
{
    const char *unixp = "ftp://user:password@host:12/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "password")  cr = FTP_OK;
    CHECK
}

void extractpasswfrurl4(void)
{
    const char *unixp = "ftp://user:password@host:12";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "password")  cr = FTP_OK;
    CHECK
}

void extractpasswfrurl5(void)
{
    const char *unixp = "ftp://alain:alain@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "alain")  cr = FTP_OK;
    CHECK
}


void extractpasswfrurl6(void)
{
    const char *unixp = "ftp://alain:@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "")  cr = FTP_OK;
    CHECK
}

void extractpasswfrurl7(void)
{
const char *unixp = "ftp://192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "")  cr = FTP_OK;
    CHECK
}


void extractpasswfrurl8(void)
{
const char *unixp = "ftp://gato:alin@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "alin")  cr = FTP_OK;
    CHECK
}

void extractpasswfrurl9(void)
{
const char *unixp = "ftp://alain@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpasswfrurl(unixp) ;
    sgIP_dbgprint("extractpasswfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "")  cr = FTP_OK;
    CHECK
}

void extractuserfrurl1(void)
{
    const char *unixp = "ftp://user:password@host:port/path";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl >%s< >%s< \n",unixp,path.c_str());
    if (path == "user")  cr = FTP_OK;
    CHECK
}


void extractuserfrurl2(void)
{
const char *unixp = "ftp://alain:alain@192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "alain")  cr = FTP_OK;
    CHECK
}


void extractuserfrurl3(void)
{
    const char *unixp = "ftp://user:password@host:12/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "user")  cr = FTP_OK;
    CHECK
}

void extractuserfrurl4(void)
{
    const char *unixp = "ftp://user:password@host:12";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "user")  cr = FTP_OK;
    CHECK
}

void extractuserfrurl5(void)
{
    const char *unixp = "ftp://:alain@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "")  cr = FTP_OK;
    CHECK
}


void extractuserfrurl6(void)
{
    const char *unixp = "ftp://alain:@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "alain")  cr = FTP_OK;
    CHECK
}

void extractuserfrurl7(void)
{
const char *unixp = "ftp://192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "")  cr = FTP_OK;
    CHECK
}


void extractuserfrurl8(void)
{
const char *unixp = "ftp://gato:alin@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "gato")  cr = FTP_OK;
    CHECK
}

void extractuserfrurl9(void)
{
const char *unixp = "ftp://alain@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractuserfrurl(unixp) ;
    sgIP_dbgprint("extractuserfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "alain")  cr = FTP_OK;
    CHECK
}



void extractpathfrurl1(void)
{
    const char *unixp = "ftp://user:password@host:port/path";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl >%s< >%s< \n",unixp,path.c_str());
    if (path == "/path")  cr = FTP_OK;
    CHECK
}


void extractpathfrurl2(void)
{
const char *unixp = "ftp://alain:alain@192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "/arm9")  cr = FTP_OK;
    CHECK
}


void extractpathfrurl3(void)
{
    const char *unixp = "ftp://user:password@host:12/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "/")  cr = FTP_OK;
    CHECK
}

void extractpathfrurl4(void)
{
    const char *unixp = "ftp://user:password@host:12";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "/")  cr = FTP_OK;
    CHECK
}

void extractpathfrurl5(void)
{
    const char *unixp = "ftp://alain:alain@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "/")  cr = FTP_OK;
    CHECK
}


void extractpathfrurl6(void)
{
    const char *unixp = "ftp://alain:alain@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "/")  cr = FTP_OK;
    CHECK
}

void extractpathfrurl7(void)
{
const char *unixp = "ftp://192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "/arm9")  cr = FTP_OK;
    CHECK
}


void extractpathfrurl8(void)
{
const char *unixp = "ftp://192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "/")  cr = FTP_OK;
    CHECK
}

void extractpathfrurl9(void)
{
const char *unixp = "ftp://alain:alain@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractpathfrurl(unixp) ;
    sgIP_dbgprint("extractpathfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "/")  cr = FTP_OK;
    CHECK
}


void extractportfrurl1(void)
{
    const char *unixp = "ftp://user:password@host:port/path";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl >%s< >%s< \n",unixp,path.c_str());
    if (path == "port")  cr = FTP_OK;
    CHECK
}


void extractportfrurl2(void)
{
const char *unixp = "ftp://alain:alain@192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "21")  cr = FTP_OK;
    CHECK
}


void extractportfrurl3(void)
{
    const char *unixp = "ftp://user:password@host:12/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "12")  cr = FTP_OK;
    CHECK
}

void extractportfrurl4(void)
{
    const char *unixp = "ftp://user:password@host:12";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "12")  cr = FTP_OK;
    CHECK
}

void extractportfrurl5(void)
{
    const char *unixp = "ftp://:alain@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "21")  cr = FTP_OK;
    CHECK
}


void extractportfrurl6(void)
{
    const char *unixp = "ftp://alain:@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "21")  cr = FTP_OK;
    CHECK
}

void extractportfrurl7(void)
{
const char *unixp = "ftp://192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "21")  cr = FTP_OK;
    CHECK
}


void extractportfrurl8(void)
{
const char *unixp = "ftp:gato:alin@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "21")  cr = FTP_OK;
    CHECK
}

void extractportfrurl9(void)
{
const char *unixp = "ftp://alain@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extractportfrurl(unixp) ;
    sgIP_dbgprint("extractportfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "21")  cr = FTP_OK;
    CHECK
}



void extracthostfrurl1(void)
{
    const char *unixp = "ftp://user:password@host:port/path";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl >%s< >%s< \n",unixp,path.c_str());
    if (path == "host")  cr = FTP_OK;
    CHECK
}


void extracthostfrurl2(void)
{
const char *unixp = "ftp://alain:alain@192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "192.168.1.2")  cr = FTP_OK;
    CHECK
}


void extracthostfrurl3(void)
{
    const char *unixp = "ftp://user:password@host:12/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "host")  cr = FTP_OK;
    CHECK
}

void extracthostfrurl4(void)
{
    const char *unixp = "ftp://user:password@host:12";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "host")  cr = FTP_OK;
    CHECK
}

void extracthostfrurl5(void)
{
    const char *unixp = "ftp://:alain@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "192.168.1.2")  cr = FTP_OK;
    CHECK
}


void extracthostfrurl6(void)
{
    const char *unixp = "ftp://alain:@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "192.168.1.2")  cr = FTP_OK;
    CHECK
}

void extracthostfrurl7(void)
{
const char *unixp = "ftp://192.168.1.2/arm9";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "192.168.1.2")  cr = FTP_OK;
    CHECK
}


void extracthostfrurl8(void)
{
const char *unixp = "ftp:gato:alin@192.168.1.2";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "192.168.1.2")  cr = FTP_OK;
    CHECK
}

void extracthostfrurl9(void)
{
const char *unixp = "ftp://alain@192.168.1.2/";
    tstring path;
    u32 cr = FTP_NOTOK;
    
    
    path = extracthostfrurl(unixp) ;
    sgIP_dbgprint("extracthostfrurl '%s' '%s' \n",unixp,path.c_str());
    if (path == "192.168.1.2")  cr = FTP_OK;
    CHECK
}


void listftp1(void)
{
    const char *fulldir = "ftp://alain:alain@192.168.1.2/";
    DIR_ITER *dirIter;
    char filename[50];
    struct stat filestat;
    int res,i;
    
    dirIter = sdiropen(fulldir); 
    //sgIP_dbgprint("sdirnext");
    for(i=0; i < 3; i++)
    {
        res = sdirnext((DIR_ITER*)1,filename,&filestat);
        if(res != 0)
        break;
        sgIP_dbgprint(">file %s type %d len %d \n",filename,filestat.st_mode, filestat.st_mtime);sleep(1);
    }
	//sgIP_dbgprint("sdirclose");
    sdirclose((DIR_ITER*)1);
    sleep(2);
}

void listftp2(void)
{
    const char *fulldir = "ftp://alain:alain@192.168.1.2/arm9";
    DIR_ITER *dirIter;
    char filename[50];
    struct stat filestat;
    int res,i;
    
    dirIter = sdiropen(fulldir);
    sleep(10);
    
    for(i=0; i < 20; i++)
    {
        res = sdirnext((DIR_ITER*)1,filename,&filestat);
        if(res != 0)
        break;
        sgIP_dbgprint(">file %s type %d len %d \n",filename,filestat.st_mode, filestat.st_mtime);sleep(2);
    }
    sdirclose((DIR_ITER*)1);
    sleep(10);
}

void sstat1(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/CATA_CLT.mht";
    struct stat filestat;
    
    cr = sstat(fulldir, &filestat) ;
    sgIP_dbgprint(">file %s type %d len %d \n",fulldir ,filestat.st_mode, filestat.st_mtime);
    sleep(10);
}


void sstat2(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/adir";
    char filename[50];
    struct stat filestat;
    
    cr = sstat(fulldir, &filestat) ;
    sgIP_dbgprint(">file %s type %d len %d \n",fulldir ,filestat.st_mode, filestat.st_mtime);
    sleep(10);
}

void sLoadFileToMem1(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/afile";
    u8 * inbuffer=0;
    u64 size;
    
    cr = sLoadFileToMem(fulldir, &inbuffer, &size);
    sgIP_dbgprint("content file %c%c%c \n" ,(*inbuffer), *(inbuffer+1), *(inbuffer+2));
    
    if (inbuffer) free(inbuffer);
    sleep(10);
}

void smkdir1(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/newdir";
    
    cr = smkdir(fulldir, 777);
    
    sgIP_dbgprint(">smkdir %s\n",fulldir);
    sleep(10);
}

void sCopyFile1(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/afile";
    const char *fulldir2 = "ftp://alain:alain@192.168.1.2/anewfile";
    
    cr = sCopyFile(fulldir, fulldir2);
    
    sgIP_dbgprint(">sCopyFile %s %s\n ",fulldir, fulldir2);
    sleep(10);
}

void sCopyFile2(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/afile";
    const char *fulldir2 = "sd:/afile";
    
    cr = sCopyFile(fulldir, fulldir2);
    
    
    sgIP_dbgprint(">sCopyFile %s %s\n ",fulldir, fulldir2);
    sleep(10);
}


void sCopyFile3(void)
{
    int cr;
    const char *fulldir = "sd:/afile";
    const char *fulldir2 = "ftp://alain:alain@192.168.1.2/anewfile2";
    
    cr = sCopyFile(fulldir, fulldir2);
    sgIP_dbgprint(">sCopyFile %s %s\n ",fulldir, fulldir2);
    
    sleep(10);
}

void srename1(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/anewfile2";
    const char *fulldir2 = "ftp://alain:alain@192.168.1.2/afilerenamed";
    
    cr = srename(fulldir, fulldir2);
    sgIP_dbgprint(">rename %s %s\n ",fulldir, fulldir2);
    sleep(10);
}

void remove1(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/afilerenamed";
    
    cr = sremove (fulldir );
    sgIP_dbgprint(">remove %s\n ",fulldir);
    sleep(10);
}

void remove2(void)
{
    int cr;
    const char *fulldir = "ftp://alain:alain@192.168.1.2/newdir";
    
    cr = sremove (fulldir );
    sgIP_dbgprint(">remove %s\n ",fulldir);
    sleep(10);
}

void sCheckFile1(void)
{
    const char *fulldir = "ftp://alain:alain@192.168.1.2/newdir";
    bool cr ;
    
    cr = sCheckFile(fulldir);
    sgIP_dbgprint(">check %s %d\n ",fulldir,cr);
    sleep(10);
}

void sCheckFile2(void)
{
    const char *fulldir = "ftp://alain:alain@192.168.1.2/afile";
    bool cr ;
    
    cr = sCheckFile(fulldir);
    sgIP_dbgprint(">check %s %d\n ",fulldir,cr);
    sleep(10);
}


int pause(void)
{
return 0;
}


void testwarp(void)
{
    
    slipfromleft1();
    slipfromleft2();
    slipfromleft3();
    slipfromleft4();
    
    slipfromright1();
    slipfromright2();
    slipfromright3();
    slipfromright4();
    
    
    extractpathfrurl1();
    extractpathfrurl2();
    extractpathfrurl4();
    extractpathfrurl5();
    extractpathfrurl6();
    extractpathfrurl7();
    extractpathfrurl8();
    extractpathfrurl9();
    
    extractpasswfrurl1();
    extractpasswfrurl2();
    extractpasswfrurl3();
    extractpasswfrurl4();
    extractpasswfrurl5();
    extractpasswfrurl6();
    extractpasswfrurl7();
    extractpasswfrurl8();
    extractpasswfrurl9();
    
    extractuserfrurl1();
    extractuserfrurl2();
    extractuserfrurl3();
    extractuserfrurl4();
    extractuserfrurl5();
    extractuserfrurl6();
    extractuserfrurl7();
    extractuserfrurl8();
    extractuserfrurl9();
    
    extracthostfrurl1();
    extracthostfrurl2();
    extracthostfrurl3();
    extracthostfrurl4();
    extracthostfrurl5();
    extracthostfrurl6();
    extracthostfrurl7();
    extracthostfrurl8();
    extracthostfrurl9();
    
    extractportfrurl1();
    extractportfrurl2();
    extractportfrurl3();
    extractportfrurl4();
    extractportfrurl5();
    extractportfrurl6();
    extractportfrurl7();
    extractportfrurl8();
    extractportfrurl9();
    
    while(1){
    listftp1();
    //extractpathfrurl2();
    listftp2();
    //sstat1();
    //sstat2();
    //sLoadFileToMem1();
    //smkdir1();
    //sCopyFile1();
    //sCopyFile2();
    sCopyFile3();
    //srename1();
    //remove1();
    //remove2();
    //sCheckFile1();
    //sCheckFile2();
}
   sgIP_dbgprint("****** END ");
 
}

