
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

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "TextOperations/TextEditor.h"
#include "libwiigui/gui_optionbrowser.h"
#include "Menus/Explorer.h"
#include "menu.h"
#include "main.h"
#include "input.h"
#include "filelist.h"
#include "FileOperations/filebrowser.h"
#include "FileOperations/fileops.h"
#include "devicemounter.h"
#include "FileStartUp/FileStartUp.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "network/networkops.h"
#include "BootHomebrew/BootHomebrew.h"
#include "Language/gettext.h"
#include "Language/LanguageBrowser.h"
#include "network/update.h"
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "sys.h"
#include <sys/dir.h>
#include <dirent.h>
#include <unistd.h>


#include "FTPClientconfig.h"
#include "FTPClientDefinements.h"
#include "FTPClient.h"
extern void Werror(char *p);


extern s32 SetTextfGlobal(const char *format, ...);
extern u32 launchServer(char *drive, int status, bool stop) ;
extern "C" void sgIP_dbgprint(char * txt, ...) ;

extern  void HaltGui();
extern void ResumeGui();


extern GuiText *ftpMainStatusText;
extern GuiText *WWWAdressText;
extern GuiText *ftpDebugText[19];
extern GuiText *NintendoAdressText ;

/*

void assert (int line, char *file, int nbr) {
char t[200];
	if (!nbr) {
		sprintf(t,"ASSERT line %d file %s nbr %d\n\r",line,file,nbr);
		werror(t);
		}
}
*/

using namespace std;
using namespace nsFTP;
extern nsFTP::CFTPClient ftpClient;



extern void debugwii2(const char *format, ...);
		extern void TestFtpClient (void);
		
static int  ClientFtp(void)
{
	int menu = MENU_NONE;
	int ret;

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png,button_png_size);
	GuiImageData btnOutlineOver(button_over_png,button_over_png_size);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	GuiText backBtnTxt(tr("Go Back"), 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -65);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetTrigger(&trigA);
	backBtn.SetEffectGrow();

	GuiText LaunchBtnTxt(tr("Launch Ftp"), 22, (GXColor){0, 0, 0, 255});
	GuiText StopBtnTxt(   tr("Stop Ftp "), 22, (GXColor){0, 0, 0, 255});
	GuiImage updateBtnImg(&btnOutline);
	GuiButton updateBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	updateBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	updateBtn.SetPosition(-50, -65);
	updateBtn.SetLabel(&LaunchBtnTxt);
	updateBtn.SetImage(&updateBtnImg);
	updateBtn.SetSoundOver(&btnSoundOver);
	updateBtn.SetTrigger(&trigA);
	updateBtn.SetEffectGrow();

	GuiImageData settingsimgData(bg_cftp_png,bg_cftp_png_size);
	GuiImage settingsimg(&settingsimgData);
	settingsimg.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	settingsimg.SetPosition(0, 0);
	u32 y =20; 
	u32 x=40;
	    
	char ftpDebugStatus[50];
    	snprintf(ftpDebugStatus, sizeof(ftpDebugStatus), "%s", "");
	
	for (u32 i=0;i<10;i++) 
	{
	ftpDebugText[i] = new GuiText(ftpDebugStatus, 20, (GXColor) {0, 0, 0, 255});
	ftpDebugText[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	ftpDebugText[i]->SetPosition(x, y*(5+i));
	}
	
	u32 font = 22;
	u32 linestodraw = 10;

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsimg);
	w.Append(ftpMainStatusText);
	w.Append(&updateBtn);
	w.Append(&backBtn);
	for (u32 i=0;i<10;i++) 
	{
	w.Append(ftpDebugText[i]);
	}


	MainWindow::Instance()->Append(&w);
    	w.SetEffect(EFFECT_FADE, 50);
	ResumeGui();

	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	while(menu == MENU_NONE)
	{
	    usleep(THREAD_SLEEP);

        if(shutdown == 1)
            Sys_Shutdown();

        else if(reset == 1)
            Sys_Reboot();

       else if(updateBtn.GetState() == STATE_CLICKED)
		{
		updateBtn.ResetState();
		TestFtpClient ();
		}
		
		else if(backBtn.GetState() == STATE_CLICKED)
		{


		menu = MENU_BROWSE_DEVICE;
		}
		
        else if(Taskbar::Instance()->GetMenu() != MENU_NONE)
			menu = Taskbar::Instance()->GetMenu();

	}

    	w.SetEffect(EFFECT_FADE, -50);
	while(w.GetEffect() > 0) usleep(THREAD_SLEEP);

	HaltGui();
	MainWindow::Instance()->Remove(&w);

	ResumeGui();

	return menu;
}


// nsFTP::TSpFTPFileStatusVector list;

tm date = {
	59,     /* seconds after the minute - [0,59] */
	59,     /* minutes after the hour - [0,59] */
	23,    /* hours since midnight - [0,23] */
	26,    /* day of the month - [1,31] */
	9,     /* months since January - [0,11] */
	1965,    /* years since 1900 */
	0,    /* days since Sunday - [0,6] */
	12,    /* days since January 1 - [0,365] */
	1   /* daylight savings time flag */
};

using namespace nsFTP;


#define CHECK if (cr==FTP_OK) sgIP_dbgprint("ftpMain ******** SUCCESS ********  %d\n",__LINE__); else {sgIP_dbgprint("******** FAILED **********  %d\n",__LINE__); sleep(10);}

extern void waitkey(void );
extern void TestFtpClient (void);

#define _P(a) (&(const tstring) a)


void 
launchFtpClient (void)
{
	//TestFtpClient ();

	tstring m_strResponse;
	long lSize;
	int cr ;

	ftpClient.Init();

	nsFTP::CLogonInfo logonInfo("192.168.1.2", 21, "alain",  "alain");
	ftpClient.Login(logonInfo);

	// cr = ftpClient.NameList(_P("/"), /* mode zip */ false, /* fPasv */ false, m_strResponse);
	// CHECK 

	const tstring tt ="/";
	cr = ftpClient.List(&tt, /* mode zip */ false, /* fPasv */ false);
	CHECK 

		CFTPFileStatus*  FileStat;

	for (unsigned int i=0; i<ftpClient.NumberOfFiles(); i++) 
	{ 
		FileStat = ftpClient.GetFileStat(i);
		sgIP_dbgprint("%d %s \n", i, FileStat->Name().c_str());
	}


	const tstring hh =".";
	char a ;
	char b;
	bool stop;
	unsigned int currfile = 0;
	while (1) {
		sgIP_dbgprint("enter command u;d;a;s;q");
toto:
		a = getchar(); if (a=='\n' || a =='\r') goto toto;

		switch (a) 
		{
		case 'u' :
			if (currfile==0) ;
			else
				currfile--;
			FileStat = ftpClient.GetFileStat(currfile);
			sgIP_dbgprint("%d %s \n", currfile, FileStat->Name().c_str());
			break;
		case 'd' :
			if (currfile+1>=ftpClient.NumberOfFiles()) ;
			else
				currfile++;
			FileStat = ftpClient.GetFileStat(currfile);
			sgIP_dbgprint("%d %s \n", currfile, FileStat->Name().c_str());
			break;
		case 's' :
			printf("Special action action\n");

			FileStat = ftpClient.GetFileStat(currfile);
			if (FileStat->IsDot()){ // Print current DIR
				sgIP_dbgprint("Its'a dot");
				cr = ftpClient.PrintWorkingDirectory();
				break;
			}
			if (FileStat->IsDDot()) { // Change to home DIR
				sgIP_dbgprint("Its'a double dot");
				cr = ftpClient.ChangeWorkingDirectory(&FileStat->Name());
				cr = ftpClient.List(&tt, /* mode zip */ false, /* fPasv */ false);
				currfile=0;
				break;
			}  
			if (FileStat->IsDirectory()) { // Change to destination DIR
				printf("Enter Directory action action\n");
toto2:
		a = getchar(); if (a=='\n' || a =='\r') goto toto2;

				// change directory
				// case delete
				switch(a)
				{
				case 'c' : // change directory
					cr = ftpClient.ChangeWorkingDirectory(&FileStat->Name());
					cr = ftpClient.List(&hh, /* mode zip */ false, /* fPasv */ false);
					currfile=0;
					break;
				case 'd' : // case delete directory
					cr = ftpClient.RemoveDirectory(&FileStat->Name());
					cr = ftpClient.List(&tt, /* mode zip */ false, /* fPasv */ false);
					currfile = ftpClient.IsCursor(currfile-1);
					break;
				}
			}
			else  {
				if (FileStat->IsFile()) { 
					printf("Enter File action action\n");
toto3:
		a = getchar(); if (a=='\n' || a =='\r') goto toto3;


					// case delete
					// case upload
					// case upload and rename
					// case rename
					switch(a)
					{
					case 'd' : // case delete file
						cr = ftpClient.Delete(&FileStat->Name());
						cr = ftpClient.List(&tt, /* mode zip */ false, /* fPasv */ false);
						currfile = ftpClient.IsCursor(currfile-1);
						break;
					case 'u' : // case upload
						cr = ftpClient.DownloadFile(&FileStat->Name(), _P("/aDetruire"), /*zip*/ true );
						break;
					case 'i' : // case upload and rename
						cr = ftpClient.DownloadFile(&FileStat->Name(), _P("/aDetruire"), /*zip*/ true );
						break;
					case 'r' : // case rename
						cr = ftpClient.Rename(&FileStat->Name(), _P("/RENAMED.mht"));
						cr = ftpClient.List(&tt, /* mode zip */ false, /* fPasv */ false);
						break;
					}
				}

			}
			break;

		case 'a' :
			printf("selection action\n");
			FileStat = ftpClient.GetFileStat(currfile);
			sgIP_dbgprint("%d %s \n", currfile, FileStat->Name().c_str());

			if (FileStat->IsDot()){ // Print current DIR
				sgIP_dbgprint("Its'a dot");
				cr = ftpClient.PrintWorkingDirectory();
				break;
			}

			if (FileStat->IsDDot()) { // Change to home DIR
				sgIP_dbgprint("Its'a double dot");
				cr = ftpClient.ChangeWorkingDirectory(&FileStat->Name());
				cr = ftpClient.List(&tt, /* mode zip */ false, /* fPasv */ false);
				currfile=0;
				// change directory
			}

			if (FileStat->IsDirectory()) { // Change to destination DIR
				sgIP_dbgprint("Its'a directory");
				cr = ftpClient.ChangeWorkingDirectory(&FileStat->Name());
				cr = ftpClient.List(&hh, /* mode zip */ false, /* fPasv */ false);
				currfile=0;
				// change directory
			}

			if (FileStat->IsFile()) { // upload file
				sgIP_dbgprint("Its'a file");
				cr = ftpClient.DownloadFile(&FileStat->Name(), _P("/aDetruire"), /*zip*/ true );
			}
		}

		if (a=='q') break;
	}


	ftpClient.Logout();
	exit(1);
}

extern void testwarp(void);


#define BLOCKSIZE               50*1024      //50KB

int LoadaFileToMem(const char *filepath, u8 **inbuffer, u64 *size)
{
    int ret;
	char temp[MAXPATHLEN];
	sprintf(temp, "%s", filepath);
	char * filename = strrchr(temp, '/')+1;

    FILE *file = fopen(filepath, "rb");

    if (file == NULL) {
        return FTP_NOTOK;
    }

    u64 filesize = FileSize(filepath);

    u8 *tempbuffer = (u8 *) malloc(BLOCKSIZE);
    if (tempbuffer == NULL) {
        fclose(file);
        return FTP_NOTOK;
    }

    u64 done = 0;

    u8 *readbuffer = (u8 *) malloc(BLOCKSIZE);
    if(!readbuffer) {
            free(tempbuffer);
            fclose(file);
            return FTP_NOTOK;
    }

    do {
        ret = fread(tempbuffer, 1, BLOCKSIZE, file);
        done += ret;

        u8 *tempreadbuffer = (u8 *) realloc(readbuffer, done);
        if(!tempreadbuffer) {
            free(tempreadbuffer);
            tempreadbuffer = NULL;
            free(readbuffer);
            readbuffer = NULL;
            free(tempbuffer);
            tempbuffer = NULL;
            fclose(file);
            return FTP_NOTOK;
        } else
            readbuffer = tempreadbuffer;

        memcpy(readbuffer+(done-ret), tempbuffer, ret);

    } while(done < filesize);

    free(tempbuffer);
    fclose(file);

    if (done != filesize) {
        free(readbuffer);
        return -3;
    }

    *inbuffer = readbuffer;
    *size = filesize;

    return FTP_OK;
}


void 
TestFtpMemClient (void)
{
	tstring m_strResponse;
	long lSize;
	int cr ;

	ftpClient.Init();

	nsFTP::CLogonInfo logonInfo("192.168.1.2", 21, "alain",  "alain");
	ftpClient.Login(logonInfo);

	u64 localfilesize = 0;
	u64 remotefilesize = 0;
	
	// download the memfile

	cr = ftpClient.DownloadFile(/* remote */ _P("/memfile.mht"), /* local */ _P("usb:/memfile.mht"), /*zip*/ false, true ); // transfert du fichier du serveur vers client
	CHECK 

    localfilesize = FileSize("usb:/memfile.mht");
	
	cr = ftpClient.FileSize( _P("/memfile.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) cr = FTP_NOTOK;
	CHECK 

    u8 * ptr = NULL;
	u64 size = 0;

	// load memfile in memory and then upload it

	cr = LoadaFileToMem("usb:/memfile.mht", &ptr, &size);
	CHECK

	cr = ftpClient.UploadMemFile((const void *)ptr, (u32) size, _P("/memfile2.mht"), /*zip*/ false, false ); // transfert du fichier du serveur vers client
	CHECK 

	cr = ftpClient.FileSize( _P("/memfile2.mht"),remotefilesize );
	CHECK

	if (size != remotefilesize) cr = FTP_NOTOK;
	CHECK 
	
    free(ptr);

	// load memfile in memory and then upload + add content for the second time

	cr = LoadaFileToMem("usb:/memfile.mht", &ptr, &size);
	CHECK

	cr = ftpClient.UploadMemFile((const void *)ptr, (u32) size, _P("/memfile2.mht"), /*zip*/ false, true ); // transfert du fichier du serveur vers client
	CHECK 

	cr = ftpClient.FileSize( _P("/memfile2.mht"),remotefilesize );
	CHECK

	if (size*2 != remotefilesize) cr = FTP_NOTOK;
	CHECK 
	
    free(ptr);


	// download in memory

	cr = ftpClient.FileSize( _P("/memfile2.mht"),remotefilesize );
	CHECK
	
	ptr = (u8 *) malloc(remotefilesize);

	u32 inmem;
	cr = ftpClient.DownloadMemFile( _P("/memfile2.mht"),(const void *)ptr, (u32) remotefilesize,  &inmem, /*zip*/ false, true ); // transfert du fichier du serveur vers client
	CHECK 

    free(ptr);

	ftpClient.Logout();
}


void 
TestDownload (void)
{
	tstring m_strResponse;
	u64 lSize;
	int cr ;

    // download the master file 
	ftpClient.Init();

	nsFTP::CLogonInfo logonInfo("192.168.1.2", 21, "alain",  "alain");
	ftpClient.Login(logonInfo);

	// withtoup zip
	sgIP_dbgprint("DOWNLOAD WITHOUT ZIP AND OVERWRITE\n") ;

	cr = ftpClient.DownloadFile(/* remote */ _P("/CATA_SVR.mht"), 
	/* local */ _P("usb:/CATA_CLT.mht"), 
	/*zip*/ false, true ); 

	CHECK 
	u64 localfilesize = FileSize("usb:/CATA_CLT.mht");
	u64 remotefilesize = 0;
		
	cr = ftpClient.FileSize( _P("/CATA_SVR.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	// with zip
	sgIP_dbgprint("DOWNLOAD WITH ZIP AND OVERWRITE \n") ;

	cr = ftpClient.DownloadFile(/* remote */ _P("/CATA_SVR.mht"), 
	/* local */ _P("usb:/CATA_CLT.mht"), 
	/*zip*/ false, true ); 

	CHECK 
	 localfilesize = FileSize("usb:/CATA_CLT.mht");
	 remotefilesize = 0;
		
	cr = ftpClient.FileSize( _P("/CATA_SVR.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 


	// with zip
	sgIP_dbgprint("DOWNLOAD WITH ZIP AND ADD CONTENT \n") ;

	cr = ftpClient.DownloadFile(/* remote */ _P("/CATA_SVR.mht"), 
	/* local */ _P("usb:/CATA_CLT.mht"), 
	/*zip*/ false, true ); // dernier en true 

	CHECK 
	 localfilesize = FileSize("usb:/CATA_CLT.mht");
	 remotefilesize = 0;
		
	cr = ftpClient.FileSize( _P("/CATA_SVR.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	ftpClient.Logout();

}

void 
TestFtpClient (void)
{
	tstring m_strResponse;
	u64 lSize;
	int cr ;


TestDownload ();

return ;

//	 testwarp();
//return;


    // download the master file 
	ftpClient.Init();

	nsFTP::CLogonInfo logonInfo("192.168.1.2", 21, "alain",  "alain");
	ftpClient.Login(logonInfo);

	cr = ftpClient.DownloadFile(/* remote */ _P("/CATA_SVR.mht"), /* local */ _P("usb:/CATA_CLT.mht"), /*zip*/ false, true ); // transfert du fichier du serveur vers client
	CHECK 
	u64 localfilesize = FileSize("usb:/CATA_CLT.mht");
	u64 remotefilesize = 0;
	
	cr = ftpClient.UploadFile(_P("usb:/CATA_CLT.mht"), _P("/TO_DELETE_1.mht"), /* zip */ true, /* over*/ false); // transfert toto du client vers serveur
	CHECK 
	
	cr = ftpClient.FileSize( _P("/TO_DELETE_1.mht"),remotefilesize );
	CHECK
	
	if (localfilesize != remotefilesize) cr = FTP_NOTOK;
	CHECK 
	
	cr = ftpClient.Delete(_P("/TO_DELETE_1.mht"));
	CHECK 

	cr = ftpClient.DownloadFile(/* remote */ _P("/CATA_SVR.mht"), /* local */ _P("usb:/CATA_CLT_1.mht"), /*zip*/ false, true ); // transfert du fichier du serveur vers client
	CHECK 

	 localfilesize = FileSize("usb:/CATA_CLT_1.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/CATA_SVR.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	cr = ftpClient.DownloadFile(_P("/CATA_SVR.mht"), _P("usb:/CATA_CLT_2.mht"), /*zip*/ true, true ); // transfert du fichier du serveur vers client
	CHECK 


	 localfilesize = FileSize("usb:/CATA_CLT_2.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/CATA_SVR.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 


	cr = ftpClient.DownloadFile(_P("/CATA_SVR.mht"), _P("usb:/CATA_CLT_3.mht"), /*zip*/ false, /* over */ false ); // transfert du fichier du serveur vers client
	CHECK 


	localfilesize = FileSize("usb:/CATA_CLT_3.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/CATA_SVR.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	cr = ftpClient.DownloadFile(_P("/CATA_SVR.mht"), _P("usb:/CATA_CLT.mht"), /*zip*/ false, /* over */ false ); // transfert du fichier du serveur vers client
	CHECK 


	 localfilesize = FileSize("usb:/CATA_CLT.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/CATA_SVR.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	cr = ftpClient.UploadFile(_P(/* local */ "usb:/CATA_CLT.mht"), /* remote */ _P("/TATA_SVR_1.mht"), /* zip */ true, /* over*/ false); // transfert toto du client vers serveur
	CHECK 


	 localfilesize = FileSize("usb:/CATA_CLT.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/TATA_SVR_1.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	cr = ftpClient.UploadFile(_P("usb:/CATA_CLT.mht"), _P("/TATA_SVR_2.mht"), /* zip */ false, /* over*/ false); // transfert toto du client vers serveur
	CHECK 


	 localfilesize = FileSize("usb:/CATA_CLT.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/TATA_SVR_2.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	cr = ftpClient.UploadFile(_P("usb:/CATA_CLT.mht"), _P("/TATA_SVR_3.mht"), /* zip */ false, /* over*/ true); // transfert toto du client vers serveur
	CHECK 


	 localfilesize = FileSize("usb:/CATA_CLT.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/TATA_SVR_3.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	ftpClient.Logout();
//	exit(1);
//
}


void 
TestVariousCmd (void)
{
	tstring m_strResponse;
	u64 lSize;
	int cr ;


	u64 localfilesize = 0;
	u64 remotefilesize = 0;


////
//	ftpClient.DownloadFile(_"/CATA.mht", "/CATO.mht", /*zip*/ true ); // transfert du fichier du serveur vers client

////
	ftpClient.Feat();
	sgIP_dbgprint("str .... %s",m_strResponse.c_str());
	CHECK 


	cr = ftpClient.UploadFile(_P("usb:/CATA_CLT.mht"), _P("/TO_RENAME.mht"), /* zip */ true, /* over*/ false); // transfert toto du client vers serveur
	CHECK 


	 localfilesize = FileSize("usb:/CATA_CLT.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/TO_RENAME.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 


	cr = ftpClient.Rename(_P("/TO_RENAME.mht"), _P("/RENAMED.mht"));
	CHECK


	 localfilesize = FileSize("usb:/CATA_CLT.mht");
	 remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/RENAMED.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 

	cr = ftpClient.Delete(_P("/RENAMED.mht"));
	CHECK 



	//cr = ftpClient.UploadFile(_P("usb:/CATA_CLT.mht"), _P("/TO_MODIF1.mht"), /* zip */ true); // transfert toto du client vers serveur
	//CHECK 
	cr = ftpClient.FileModificationTime(_P("/TO_MODIF1.mht"),date);
	CHECK 

	cr = ftpClient.FileSize(_P("/TO_STAT_1.mht"),lSize);
	CHECK 
////

		const tstring tt ="/";
	cr = ftpClient.List(&tt, /* mode zip */ false, /* fPasv */ true);
	CHECK 

	if (cr==FTP_OK){
		CFTPFileStatus*  FileStat;
	int k=1;
	for (unsigned int i=0; i<ftpClient.NumberOfFiles(); i++) 
	{ 
	FileStat = ftpClient.GetFileStat(i);
		sgIP_dbgprint("%d %s \n", k++, FileStat->Name().c_str());
	}
	}
	cr = ftpClient.MakeDirectory(_P("/DIRECT_SRV"));
	CHECK
	
	cr = ftpClient.FileSize( _P("/DIRECT_SRV"),remotefilesize );
	CHECK

	if (0 != remotefilesize) cr = FTP_NOTOK;
	CHECK 


	cr = ftpClient.ChangeWorkingDirectory(_P("/DIRECT_SRV"));
	CHECK 
	cr = ftpClient.ChangeWorkingDirectory(_P("/"));
	CHECK 
	cr = ftpClient.RemoveDirectory(_P("/DIRECT_SRV"));
	CHECK 


	cr = ftpClient.PrintWorkingDirectory();
	CHECK 
	cr = ftpClient.System();
	CHECK 
	cr = ftpClient.Noop();
	CHECK 
	cr = ftpClient.Status(_P(""));
	CHECK 
	cr = ftpClient.UploadFile(_P("usb:/CATA_CLT.mht"), _P("/TO_STAT_1.mht"), /* zip */ true, /* over*/ false); // transfert toto du client vers serveur
	CHECK 


	localfilesize = FileSize("usb:/CATA_CLT.mht");
	remotefilesize = 0;

	
	cr = ftpClient.FileSize( _P("/TO_STAT_1.mht"),remotefilesize );
	CHECK

	if (localfilesize != remotefilesize) { 
	cr = FTP_NOTOK;
	sgIP_dbgprint("CMP %lu %d \n",localfilesize,remotefilesize) ;
	}
	CHECK 
	ftpClient.Logout();
//	exit(1);

//
}
