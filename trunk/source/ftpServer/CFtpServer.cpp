/****************************************************************************
 * Copyright (C) 2009
 * by LilouMaster
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

#include <errno.h>
#include <network.h>

#include <malloc.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/iosupport.h>
#include <network.h>
#include <ogcsys.h>
#include <ogc/lwp_watchdog.h>
#include <ogc/mutex.h>
#include <network.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <sys/fcntl.h>

#include "getopt.h"
#include <fat.h>

#include <di/di.h>
#include <network.h>
#include <ogc/lwp_watchdog.h>
#include <string.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include <network.h>

#include "CFtpServer.h"
#include "csString.h"
#include "csConfig.h"

#include "CFtpServerGlobal.h"
#include "CFtpServerConfig.h"
//#include "console/GUI_console.h"
#include "vrt/vrt.h"
#include "main.h"
#include "Language/gettext.h"

#define NET_READ_BUFFERSIZE                     7300
#define NET_WRITE_BUFFERSIZE                    4096
#define NET_TIMEOUT                             10000  // network connection timeout, in ms


extern void sgIP_dbgprint(const char * txt, ...) ;
extern void sgIP_appprint(const char * txt, ...) ;

#define INVALID_HANDLE_VALUE NULL

CFtpServer FtpServer;


extern void debug(const char * text);

void dummy(const char *format, ...)
{
}
char *mypszPath;

extern void dbgprintclientIP(const char * text);
extern void dbgprintWiiIP(const char * text);

#define MAX_TransferBufferSize 31 * 1024
#define CFTPSERVER_REPLY_MAX_LEN (MAX_PATH+64)
#define CFTPSERVER_LIST_MAX_LINE_LEN (MAX_PATH+57)


#define dbgprintbtm sgIP_appprint
#define dbgprintbtmat sgIP_appprint
#define sgIP_dbgprint sgIP_dbgprint

#define dbgprintclient_R sgIP_appprint
#define dbgprintclient_S sgIP_appprint
#define dbgprintclient_E sgIP_appprint
#define dbgprintserver   sgIP_appprint



char TransferBufferSize[MAX_TransferBufferSize];
char TransferBufferZipSize[MAX_TransferBufferSize];

char CFTPSERVER_LIST [ CFTPSERVER_LIST_MAX_LINE_LEN ];
CFtpServer::CEnumFileInfo CEnumFile;



void CFtpServer::Create(void)
{
	usListeningPort = 21; // By default, the FTP control port is 21
	bIsListening = bIsAccepting = false;

	DataPortRange.usStart = 100; // TCP Ports [100;999].
	DataPortRange.usLen = 900;

	ulNoTransferTimeout = ulNoLoginTimeout = 0; // No timeout.
	uiCheckPassDelay = 0; // No pass delay.
	uiMaxPasswordTries = 3; // 3 pass tries before the client gets kicked.

	uiTransferBufferSize = MAX_TransferBufferSize;

	srand((unsigned) time(NULL));



#ifdef CFTPSERVER_ENABLE_EVENTS
	_OnUserEventCb = NULL;
	_OnClientEventCb = NULL;
	_OnServerEventCb = NULL;
#endif

#ifdef CFTPSERVER_ENABLE_ZLIB
	bEnableZlib = false;
#endif
	bEnableFXP = false;
}



////////////////////////////////////////
// CONFIG
////////////////////////////////////////

bool CFtpServer::SetDataPortRange( u16 usStart, u16 usLen )
{
	if( usLen != 0 && usStart > 0 && (usStart + usLen) <= 65535 ) {
		{
			DataPortRange.usStart = usStart;
			DataPortRange.usLen = usLen;
		}
		return true;
	}
	return false;
}

bool CFtpServer::GetDataPortRange( u16 *usStart, u16 *usLen )
{
	if( usStart && usLen ) {
		{
			*usStart = this->DataPortRange.usStart;
			*usLen = this->DataPortRange.usLen;
		}
		return true;
	}
	return false;
}

////////////////////////////////////////
// [START / STOP] THE SERVER
////////////////////////////////////////



bool CFtpServer::StartListening( unsigned long ulAddr, u16 usPort )
{
	if( ulAddr == INADDR_NONE || usPort == 0 )
		return false;

	OnServerEventCb( START_LISTENING );


	sgIP_dbgprint("Lstening...%d ", usPort);

	m_apSckControlConnection.Init(CUnBlockingSocket::ApplicationSocket);

	bool cr = m_apSckControlConnection.Create(SOCK_STREAM);

	if (cr==false) {
		return cr;
	}

	CSockAddr csaAddressTemp((unsigned long int) (INADDR_ANY), usPort);

	if( m_apSckControlConnection.Bind(csaAddressTemp)) {
			if (m_apSckControlConnection.Listen()){
							return true;
			}
	}

	m_apSckControlConnection.Cleanup();
	OnServerEventCb( STOP_LISTENING  );

	return false;
}

bool CFtpServer::StartAccepting( void)
{
	OnServerEventCb( START_ACCEPTING );

	if( m_apSckControlConnection.Accept() ) {
		OnServerEventCb( NEW_CLIENT );
	return true;
	}
	return false;
}


bool CFtpServer::StopListening( void )
{
	m_apSckControlConnection.Cleanup();
	OnServerEventCb( STOP_LISTENING  );

	return true;
}



bool CFtpServer::RunClient( void )
{

	sgIP_dbgprint("Startnet_accepting...");
	class CFtpServer *pFtpServer = ( class CFtpServer* ) this;

	pFtpServer->ModifyClient( &m_apSckControlConnection );

	return true;
}


void CFtpServer::CClientEntry::RunClientEx(void)
{
	this->SendReply( "220 Browser Ftp Server." );

	this->bIsCtrlCanalOpen = true;
	this->eStatus = WAITING;

}


u32 CFtpServer::CClientEntry::RunClientShell(void)
{
if ( this->Shell ()  == false)
{
	this->m_ControlConnection.Cleanup();
	return false;
}
return true;
}



////////////////////////////////////////
// FILE
////////////////////////////////////////

char* CFtpServer::CClientEntry::BuildPath( char* pszAskedPath, char **pszVirtualPath /* = NULL */ )
{
	char *pszBuffer = &localClientPath[0];

		char *pszVirtualP = BuildVirtualPath( pszAskedPath );
		if( pszVirtualP ) {

			if( snprintf( pszBuffer, MAX_PATH + 4, "%s/%s",
				OneUser.GetStartDirectory(), pszVirtualP ) > 0 )
			{
				sgIP_dbgprint("BuildVirtualPath on %s...", pszBuffer);
				SimplifyPath( pszBuffer );
				sgIP_dbgprint("simplified path on %s...", pszBuffer);

				if( strlen( pszBuffer ) <= MAX_PATH ) {
					if( pszVirtualPath ) {
						*pszVirtualPath = pszVirtualP;
					} else
						delete [] pszVirtualP;
					return pszBuffer;
				}
			}
		}


	return pszBuffer;
}

char* CFtpServer::CClientEntry::BuildVirtualPath( char* pszAskedPath )
{
	char *pszBuffer = new char[ MAX_PATH + 4 ];
	if( pszBuffer ) {

		bool bErr = false;
		if( pszAskedPath ) {
			if( pszAskedPath[ 0 ] == '/' ) {
				CFtpServer::SimplifyPath( pszAskedPath );
				if( strlen( pszAskedPath ) <= MAX_PATH ) {
					strcpy( pszBuffer, pszAskedPath );
				} else
					bErr = true;
			} else {
				if( snprintf( pszBuffer, MAX_PATH + 4, "%s/%s",
						GetWorkingDirectory(), pszAskedPath ) > 0 )
				{
					SimplifyPath( pszBuffer );
					if( strlen( pszBuffer ) > MAX_PATH )
						bErr = true;
				}
			}
		} else
			strcpy( pszBuffer, GetWorkingDirectory() );

		if( bErr == false && strlen( pszBuffer ) <= MAX_PATH ) {
			return pszBuffer;
		} else
			delete [] pszBuffer;

	} else
		pFtpServer->OnServerEventCb( MEM_ERROR );

	return NULL;
}

bool CFtpServer::SimplifyPath( char *pszPath )
{
	sgIP_dbgprint("SimplifyPath %s...",pszPath);

	if( !pszPath || *pszPath == 0 )
		return false;

	char *a;
	for( a = pszPath; *a != '\0'; ++a ) // Replace all '\' by a '/'
		if( *a == '\\' ) *a = '/';
	while( ( a = strstr( pszPath, "//") ) != NULL ) // Replace all "//" by "/"
		memmove( a, a + 1 , strlen( a ) );
	while( ( a = strstr( pszPath, "/./" ) ) != NULL ) // Replace all "/./" by "/"
		memmove( a, a + 2, strlen( a + 1 ) );
	if( !strncmp( pszPath, "./", 2 ) )
		memmove( pszPath, pszPath + 1, strlen( pszPath ) );
	if( !strncmp( pszPath, "../", 3) )
		memmove( pszPath, pszPath + 2, strlen( pszPath + 1 ) );
	if( ( a = strstr( pszPath, "/." ) ) && a[ 2 ] == '\0' )
		a[ 1 ] = '\0';

	a = pszPath; // Remove the ending '.' in case the path ends with "/."
	while( ( a = strstr( a, "/." ) ) != NULL ) {
		if( a[ 2 ] == '\0' ) {
			a[ 1 ] = '\0';
		} else
			a++;
	}

	a = pszPath; char *b = NULL;
	while( a && (a = strstr( a, "/..")) ) {
		if( a[3] == '/' || a[3] == '\0' ) {
			if( a == pszPath ) {
				if( a[3] == '/' ) {
					memmove( pszPath, pszPath + 3, strlen( pszPath + 2 ) );
				} else {
					pszPath[ 1 ] = '\0';
					break;
				}
			} else {
				b = a;
				while( b != pszPath ) {
					--b;
					if( *b == '/' ) {
						if( b == pszPath
							|| ( b == ( pszPath + 2 ) && isalpha( pszPath[ 0 ] ) && pszPath[ 1 ] == ':' ) // C:
						) {
							if( a[ 3 ] == '/' ) { // e.g. '/foo/../' or 'C:/lol/../'
								memmove( b, a + 3, strlen( a + 3 ) + 1 ); // +1 for the \0
							} else // e.g. '/foo/..' 'C:/foo/..'
								b[ 1 ] = '\0';
						} else
							memmove( b, a + 3, strlen( a + 2 ) );
						a = strstr( pszPath, "/.." );
						break;
					} else if( b == pszPath ) {
						if( a[ 3 ] == '/' ) { // e.g. C:/../
							memmove( a, a + 3, strlen( a + 3 ) + 1 ); // +1 for the \0
						} else // e.g. C:/..
							a[ 1 ] = '\0';
						a = strstr( pszPath, "/.." );
					}
				}
			}
		} else
			++a;
	}

	// Remove the ending '/'
	int iPathLen = strlen( pszPath );
	if( isalpha( pszPath[0] ) && pszPath[1] == ':' && pszPath[2] == '/' ) {
		if( iPathLen > 3 && pszPath[ iPathLen -1 ] == '/' ) { // "C:/some/path/"
			pszPath[ iPathLen - 1 ] = '\0';
			--iPathLen;
		}
	} else {
		if( pszPath[ 0 ] == '/' ) {
			if( iPathLen > 1 && pszPath[ iPathLen - 1 ] == '/' ) {
				pszPath[ iPathLen - 1 ] = '\0'; // "/some/path/"
				--iPathLen;
			}
		} else if( iPathLen > 2 && pszPath[ iPathLen - 1 ] == '/' && pszPath[ iPathLen - 2 ] != ':' ) {
			pszPath[ iPathLen - 1 ] = '\0'; // "some/path/" except sd:/
			--iPathLen;
		}
	}

	if( *pszPath == 0 || !strcmp( pszPath, ".." ) || !strcmp( pszPath, "." ) )
		strcpy( pszPath, "/" );

	sgIP_dbgprint("Simplify end ...\n");

	return true;
}

////////////////////////////////////////
// USER
////////////////////////////////////////

void CFtpServer::UpdateUser(  char *pszLogin,  char *pszPass,  char *pszStartDir,  const char *drive )
{
	if( pszLogin && pszStartDir
		&& strlen( pszLogin ) <= MaxLoginLen
		&& strlen( pszStartDir ) <= MaxRootPathLen
		&& ( !pszPass || strlen( pszPass ) <= MaxPasswordLen )	)
	{
		char *pszSDEx = strdup( pszStartDir );
		if( !pszSDEx ) return ;
		SimplifyPath( pszSDEx );

		struct stat st; // Verify that the StartPath exist, and is a directory
		sgIP_dbgprint("UpdateUser on %s...", pszSDEx);

		if( vrt_stat( pszSDEx, &st ) != 0 || !S_ISDIR( st.st_mode ) ) {
			if ( vrt_stat( pszSDEx, &st ) != 0 ) ; sgIP_dbgprint("vrt_stat...");
			free( pszSDEx );
		}
		sgIP_dbgprint("Is DIR on %s...", pszSDEx);

		strcpy( OneUser.szLogin, pszLogin );
		strcpy( OneUser.szStartDirectory, pszSDEx );
		free( pszSDEx );
		if (pszPass) strcpy(OneUser.szPassword, pszPass);

		OneUser.bIsEnabled = true;

		OnUserEventCb( NEW_USER, &OneUser, NULL );

	}
}


////////////////////////////////////////
// CLIENT
////////////////////////////////////////


void CFtpServer::ModifyClient( CUnBlockingSocket *controlSocket )
{

		OneClient.Init();

		OneClient.m_ControlConnection = *controlSocket;

		OneClient.ulServerIP = net_gethostip();
		OneClient.ulClientIP = OneClient.m_ControlConnection.GetIPAddr();

		OneClient.pFtpServer = this;

		strcpy( OneClient.szWorkingDir, "/" );

		OnClientEventCb( NEW_CLIENT, &OneClient );

}

//////////////////////////////////////////////////////////////////////
// CFtpServer::CUserEntry CLASS
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
// CONSTRUCTOR
////////////////////////////////////////

void CFtpServer::CUserEntry::CUserEntry::Init()
{
	bDelete = false;
	bIsEnabled = false;
	ucPrivileges = 0;
	#ifdef CFTPSERVER_ENABLE_EXTRACMD
		ucExtraCommand = 0;
	#endif
	*szLogin = *szPassword = *szStartDirectory = '\0';
	pFtpServer=&FtpServer;
}

////////////////////////////////////////
// CONFIG
////////////////////////////////////////

#ifdef CFTPSERVER_ENABLE_EXTRACMD
bool CFtpServer::CUserEntry::SetExtraCommand( unsigned char ucExtraCmd )
{
	if( ( ucExtraCmd & ~( ExtraCmd_EXEC ) ) == 0 ) {
		ucExtraCommand = ucExtraCmd;
		return true;
	}
	return false;
}
#endif

bool CFtpServer::CUserEntry::SetPrivileges( unsigned char ucPriv )
{
	if( (ucPriv & ~(READFILE|WRITEFILE|DELETEFILE|LIST|CREATEDIR|DELETEDIR)) == 0) {
		ucPrivileges = ucPriv;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////
// CFtpServer::CClientEntry CLASS
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////
// CONSTRUCTOR / DESTRUCTOR
////////////////////////////////////////



bool CFtpServer::CClientEntry::Init()
{
	iZlibLevel = 0;
	pFtpServer = NULL;
	bIsLogged = false;
	eStatus = WAITING;
	eDataType = ASCII;
	eDataMode = STREAM;
	nPasswordTries = 0;
	eDataConnection = NONE;
	bIsCtrlCanalOpen = false;
	ulDataIp = usDataPort = 0;
	ulServerIP = ulClientIP = 0;
	pszCmdArg = psNextCmd = NULL;
	*szWorkingDir = *szRenameFromPath = '\0';
	iCmdLen = iCmdRecvdLen = nRemainingCharToParse = 0;
	CurrentTransfer.RestartAt = 0;

	pFtpServer=&FtpServer;
	lastcmd = time(NULL);

#ifdef CFTPSERVER_ENABLE_ZLIB
	iZlibLevel = 6; // Default Zlib compression level.
#endif

	return true;
}




void CFtpServer::CClientEntry::Delete(void)
{
	pFtpServer->OnClientEventCb( DELETE_CLIENT, this );
}


bool CFtpServer::CClientEntry::InitDelete()
{
m_ControlConnection.Cleanup();
m_DataConnection.Cleanup();

return true;
}

////////////////////////////////////////
// PRIVILEGES
////////////////////////////////////////

bool CFtpServer::CClientEntry::CheckPrivileges( unsigned char ucPriv ) const
{
	return ( bIsLogged && (ucPriv & OneUser.ucPrivileges ) == ucPriv ) ? true : false;
}

////////////////////////////////////////
// SHELL
////////////////////////////////////////

bool CFtpServer::CClientEntry::Shell( void )
{
	struct stat st;
	int nCmd;
	char *pszCmdArg=NULL;
	s32 cr;

	CFtpServer *pFtpServer = OneClient.pFtpServer;

	cr = OneClient.ReceiveLine() ;

	if (cr==ERROR) return false;

	if (cr==RETRY && timercmdstart && time(NULL) - lastcmd  > 20 ) return false; // master time-out

	if (cr==RETRY ) { timercmdstart = true ; return true; }

	lastcmd = time(NULL);
	timercmdstart = false;

	nCmd = OneClient.ParseLine();
	pszCmdArg = OneClient.pszCmdArg;

		if( nCmd == CMD_NONE ) {

			OneClient.SendReply( "500 Command not understood." );
			return true;

		} else if( nCmd == CMD_QUIT ) {

			OneClient.SendReply( "221 Goodbye." );
			pFtpServer->OnClientEventCb( CLIENT_DISCONNECT, &OneClient );
			return false;

		} else if( nCmd == CMD_USER ) {

			if( OneClient.bIsLogged == true )
				OneClient.LogOut();
			if( !pszCmdArg ) {
				OneClient.SendReply( "501 Invalid number of arguments." );
			} else {
					if( OneUser.szPassword[0] == '0' ) {
						OneClient.LogIn();
					} else
						OneClient.SendReply( "331 Password required for this user." );
				}
			return true;

		} else if( nCmd == CMD_PASS ) {

			if( OneClient.bIsLogged == true ) {
				OneClient.SendReply( "230 User Logged In." );
			} else {
				if( pFtpServer->GetCheckPassDelay() ) ; // sleep_msec( pFtpServer->GetCheckPassDelay() );
				{
					if( OneUser.bIsEnabled ) { // User is valid & enabled
						if( ( pszCmdArg && !strcmp( pszCmdArg, OneUser.szPassword ) )
							|| !*OneUser.szPassword ) // Password is valid.
						{
								OneClient.LogIn();
						} else if( !pszCmdArg )
							OneClient.SendReply( "501 Invalid number of arguments." );
					}
				}
				if( OneClient.bIsLogged ) {
					pFtpServer->OnClientEventCb( CLIENT_AUTH, &OneClient );
					return true;
				}
			}
			if( pFtpServer->GetMaxPasswordTries() && OneClient.nPasswordTries >= pFtpServer->GetMaxPasswordTries() ) {
				pFtpServer->OnClientEventCb( TOO_MANY_PASS_TRIES, &OneClient );
				return true;
			}
			OneClient.SendReply( "530 Login or password incorrect!" );
			return true;

		} else if( OneClient.bIsLogged == false ) {

			OneClient.SendReply( "530 Please login with USER and PASS." );
			return true;

		} else if( nCmd == CMD_NOOP || nCmd == CMD_ALLO ) {

			OneClient.SendReply( "200 NOOP Command Successful." );
			return true;

		#ifdef CFTPSERVER_ENABLE_EXTRACMD
		} else if( nCmd == CMD_SITE ) {

			char *pszSiteArg = pszCmdArg;
			if( pszCmdArg ) {
				unsigned char ucExtraCmd = OneUser.GetExtraCommand();
				if( !strncasecmp( pszCmdArg, "EXEC ", 5 ) ) {
					if( (ucExtraCmd & ExtraCmd_EXEC) == ExtraCmd_EXEC ) {
						if( strlen( pszSiteArg ) >= 5 )
							system( pszSiteArg + 5 );
							OneClient.SendReply( "200 SITE EXEC Successful." );
						return true;
					} else OneClient.SendReply( "550 Permission denied." );
				}
			}
			OneClient.SendReply( "501 Syntax error in arguments." );
			return true;
		} else if( nCmd == CMD_EXIT ) {
			OneClient.SendReply( "200 EXIT Command Successful." );
			return true;
		#endif
		} else if( nCmd == CMD_HELP ) {

			OneClient.SendReply( "500 No Help Available." );
			return true;

		} else if( nCmd == CMD_SYST ) {

			OneClient.SendReply( "215 UNIX Type: L8" );
			return true;

		} else if( nCmd == CMD_CLNT ) {

			if( pszCmdArg ) {
				pFtpServer->OnClientEventCb( CLIENT_SOFTWARE, &OneClient, pszCmdArg );
				OneClient.SendReply( "200 Ok." );
			} else
				OneClient.SendReply( "501 Invalid number of arguments." );
			return true;

		} else if( nCmd == CMD_STRU ) {

			if( pszCmdArg ) {
				if( !strcmp( pszCmdArg, "F" ) ) {
					OneClient.SendReply( "200 STRU Command Successful." );
				} else
					OneClient.SendReply( "504 STRU failled. Parametre not implemented" );
			} else
				OneClient.SendReply( "501 Invalid number of arguments." );
			return true;

		} else if( nCmd == CMD_OPTS ) {

			if( pszCmdArg ) {
				if( !strncasecmp( pszCmdArg, "mode z level ", 13 ) ) {
					int iLevel = atoi( pszCmdArg + 13 );
					if( iLevel > 0 && iLevel <= 9 ) {
						OneClient.iZlibLevel = iLevel;
						OneClient.SendReply( "200 MODE Z LEVEL successfully set." );
					} else
						OneClient.SendReply( "501 Invalid MODE Z LEVEL value." );
				} else
					OneClient.SendReply( "501 Option not understood." );
			} else
				OneClient.SendReply( "501 Invalid number of arguments." );
			return true;

		} else if( nCmd == CMD_MODE ) {

			if( pszCmdArg ) {
				if( !strcmp( pszCmdArg, "S" ) ) { // Stream
					OneClient.eDataMode = STREAM;
					OneClient.SendReply( "200 MODE set to S." );
				} else if( !strcmp( pszCmdArg, "Z" ) ) {
					if( pFtpServer->IsModeZEnabled() ) {
						OneClient.eDataMode = ZLIB;
						OneClient.SendReply( "200 MODE set to Z." );
					} else
						OneClient.SendReply( "504 MODE Z disabled." );
				} else if( !strcmp( pszCmdArg, "Z" ) ) {
					OneClient.SendReply( "502 MODE Z non-implemented." );
				} else
					OneClient.SendReply2( "504 \"%s\": Unsupported transfer MODE.", pszCmdArg );
			} else
				OneClient.SendReply( "501 Invalid number of arguments." );
			return true;

		} else if( nCmd == CMD_TYPE ) {

			if( pszCmdArg ) {
				if( pszCmdArg[0] == 'A' ) {
					OneClient.eDataType = ASCII; // Infact, ASCII mode isn't supported..
					OneClient.SendReply( "200 ASCII transfer mode active." );
				} else if( pszCmdArg[0] == 'I' ) {
					OneClient.eDataType = BINARY;
					OneClient.SendReply( "200 Binary transfer mode active." );
				} else
					OneClient.SendReply( "550 Error - unknown binary mode." );
			} else
				OneClient.SendReply( "501 Invalid number of arguments." );
			return true;

		} else if( nCmd == CMD_FEAT ) {

			char szFeat[ 73 ] = "211-Features:\r\n"
				" CLNT\r\n"
				" MDTM\r\n";
			if( pFtpServer->IsModeZEnabled() )
				strcat( szFeat, " MODE Z\r\n" );
			strcat( szFeat, " REST STREAM\r\n"
				" SIZE\r\n"
				"211 End" );
			OneClient.SendReply( szFeat );
			return true;

		} else if( nCmd == CMD_PORT ) {

			bool format = false;
			u32 iIp1 = 0, iIp2 = 0, iIp3 = 0, iIp4 = 0, iPort1 = 0, iPort2 = 0;
			unsigned long ulPortIp ;

			if (sscanf( pszCmdArg, "%u,%u,%u,%u,%u,%u", &iIp1, &iIp2, &iIp3, &iIp4, &iPort1, &iPort2 ) == 6)
			  format= true;
			else
			{ // |2|::1|3810|
			iIp1 = 0, iIp2 = 0, iIp3 = 0, iIp4 = 0, iPort1 = 0, iPort2 = 0;
			char dum;
			if (sscanf( pszCmdArg, "%c%u%c%c%c%u%c%u%c", &dum,&iIp1,&dum,&dum,&dum, &iPort1, &dum,&iPort2,&dum ) == 9)
			{
			  ulPortIp = OneClient.ulClientIP;
			  format = true;
			}
			else
			  format = false;
			}
			sgIP_dbgprint(">>>%s<<<<",pszCmdArg);

			if( OneClient.eStatus == WAITING && pszCmdArg
			    && format == true
				&& iIp1 <= 255 && iIp2 <= 255 && iIp3 <= 255 && iIp4 <= 255
				/* && iPort1 <= 255 && iPort2 <= 255 */ && (iIp1|iIp2|iIp3|iIp4) != 0
				&& ( iPort1 | iPort2 ) != 0 )
			{
				if( OneClient.eDataConnection != NONE )
					OneClient.ResetDataConnection();

				char szClientIP[32];
				sprintf( szClientIP, "%u.%u.%u.%u", iIp1, iIp2, iIp3, iIp4 );
				ulPortIp = inet_addr( szClientIP );

				if( !pFtpServer->IsFXPEnabled() ||
					( ulPortIp == OneClient.ulClientIP || OneClient.ulClientIP == inet_addr( "127.0.0.1" ) )
				) {
					OneClient.eDataConnection = PORT;
					OneClient.ulDataIp = ulPortIp;
					OneClient.usDataPort = (unsigned short)(( iPort1 * 256 ) + iPort2);
					OneClient.SendReply( "200 PORT command successful." );
				} else
				{
					sgIP_dbgprint("port 1 port client 2 %d,%d",ulPortIp,OneClient.ulClientIP);
					OneClient.SendReply( "501 PORT address does not match originator." );
				}

			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_PASV ) {

			if( OneClient.eStatus == WAITING ) {

				if( OneClient.eDataConnection != NONE )
					OneClient.ResetDataConnection();

			OneClient.m_DataConnection.Init(CUnBlockingSocket::DataSocket);

			if (OneClient.m_DataConnection.Create(SOCK_STREAM))
			{
			    u16 usStart, usLen;
				pFtpServer->GetDataPortRange( &usStart, &usLen );
				CSockAddr csaAddressTemp((unsigned long int) (INADDR_ANY), usStart);

				if( OneClient.m_DataConnection.Bind(csaAddressTemp))
				{
					OneClient.usDataPort=csaAddressTemp.Port();

					if (OneClient.m_DataConnection.Listen()){

					unsigned long ulIp = ntohl( OneClient.ulServerIP );
					OneClient.SendReply2( "227 Entering Passive Mode (%lu,%lu,%lu,%lu,%u,%u)",
						(ulIp >> 24) & 255, (ulIp >> 16) & 255, (ulIp >> 8) & 255, ulIp & 255,
						OneClient.usDataPort / 256 , OneClient.usDataPort % 256 );

					u32 ip =ulIp;
					struct in_addr addr;
				    addr.s_addr = ip;

					u32 port = OneClient.usDataPort;

					sgIP_dbgprint("Listening for data connections at %s:%u...\n", inet_ntoa(addr), port);
					sgIP_dbgprint("Entering Passive Mode (%u,%u,%u,%u,%u,%u)\n", (ip >> 24) & 0xff,
					(ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff, (port >> 8) & 0xff, port & 0xff);
					OneClient.eDataConnection = PASV;
					return true;
					}
				}
			}
			OneClient.m_DataConnection.Cleanup();
		  }
		} else if( nCmd == CMD_LIST || nCmd == CMD_NLST || nCmd == CMD_STAT ) {

			if( nCmd == CMD_STAT && !pszCmdArg ) {
				OneClient.SendReply( "211 :: CFtpServer / Browser FTP Server" );
				return true;
			}
			if( !OneClient.CheckPrivileges( CFtpServer::LIST ) ) {
				OneClient.SendReply( "550 Permission denied." );
				return true;
			}
			if( OneClient.eStatus != WAITING ) {
				OneClient.SendReply( "425 You're already connected." );
				return true;
			}
			memset( &OneClient.CurrentTransfer, 0x0, sizeof( OneClient.CurrentTransfer ) );

			char *pszArg = pszCmdArg; // Extract arguments
			if( pszCmdArg ) {
				while( *pszArg == '-' ) {
					while ( ++pszArg && isalnum((unsigned char) *pszArg) ) {
						switch( *pszArg ) {
							case 'a':
								OneClient.CurrentTransfer.opt_a = true;
								break;
							case 'd':
								OneClient.CurrentTransfer.opt_d = true;
								break;
							case 'F':
								OneClient.CurrentTransfer.opt_F = true;
								break;
							case 'l':
								OneClient.CurrentTransfer.opt_l = true;
								break;
						}
					}
					while( isspace( (unsigned char) *pszArg ) )
						++pszArg;
				}
			}

			mypszPath = OneClient.BuildPath( pszArg );
			if( mypszPath && vrt_stat( mypszPath, &st ) == 0 ) {

				memcpy( &OneClient.CurrentTransfer.szPath, mypszPath, strlen( mypszPath ) );
				memcpy( &OneClient.CurrentTransfer.st, &st, sizeof( st ) );
				if( nCmd == CMD_STAT  ) {
					OneClient.SendReply( "213-Status follows:" );
				} else {
					if( OneClient.OpenDataConnection( nCmd ) == false )
						return true;
				}

				OneClient.eStatus = LISTING;
				OneClient.CurrentTransfer.nCmd = nCmd;
				OneClient.CurrentTransfer.iZlibLevel = OneClient.iZlibLevel;
				OneClient.CurrentTransfer.eDataMode = OneClient.eDataMode;
				memcpy( &OneClient.CurrentTransfer.szPath, mypszPath, strlen( mypszPath ) );
				sgIP_dbgprint("PATH listThread %s", OneClient.CurrentTransfer.szPath);
				pFtpServer->OnClientEventCb( CLIENT_LIST, &OneClient, mypszPath );

				ListThread();
			}
			return true;

		} else if( nCmd == CMD_CWD || nCmd == CMD_XCWD ) {

			if( pszCmdArg ) {
				char *pszVirtualPath = NULL;
				mypszPath = OneClient.BuildPath( pszCmdArg, &pszVirtualPath );
				if( mypszPath && vrt_stat( mypszPath, &st ) == 0 && S_ISDIR( st.st_mode ) ) {
					strcpy( OneClient.szWorkingDir, pszVirtualPath );
					delete [] pszVirtualPath;
					OneClient.SendReply( "250 CWD command successful." );
					pFtpServer->OnClientEventCb( CLIENT_CHANGE_DIR, &OneClient, mypszPath );
				} else
					OneClient.SendReply( "550 No such file or directory.") ;
			} else
				OneClient.SendReply( "501 Invalid number of arguments." );
			return true;

		} else if( nCmd == CMD_MDTM ) {

			if( pszCmdArg ) {
				mypszPath = OneClient.BuildPath( &pszCmdArg[15] );
				struct tm *t;
				dbgprintbtm("cmd %s %d %d \n",mypszPath, vrt_stat( mypszPath, &st ),gmtime((time_t *) &(st.st_mtime)));
				if( mypszPath && !vrt_stat( mypszPath, &st ) && (t = gmtime((time_t *) &(st.st_mtime))) ) {
					OneClient.SendReply2( "213 %04d%02d%02d%02d%02d%02d",
						t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
						t->tm_hour, t->tm_min, t->tm_sec );
				} else
					OneClient.SendReply( "550 No such file or directory." );
			} else
				OneClient.SendReply( "501 Invalid number of arguments." );
			return true;

		} else if( nCmd == CMD_PWD || nCmd == CMD_XPWD ) {

			OneClient.SendReply2( "257 \"%s\" is current directory.", OneClient.szWorkingDir);
			return true;

		} else if( nCmd == CMD_CDUP || nCmd == CMD_XCUP ) {

			strncat( OneClient.szWorkingDir, "/..", 3 );
			pFtpServer->SimplifyPath( OneClient.szWorkingDir );
			OneClient.SendReply( "250 CDUP command successful." );
			return true;

		} else if( nCmd == CMD_ABOR ) {

			if( OneClient.eStatus != WAITING ) {
				OneClient.ResetDataConnection();
				OneClient.SendReply( "426 Previous command has been finished abnormally." );
			}
			OneClient.SendReply( "226 ABOR command successful." );
			return true;

		} else if( nCmd == CMD_REST ) {

			if( pszCmdArg && OneClient.eStatus == WAITING ) {
				#ifdef __USE_FILE_OFFSET64
					OneClient.CurrentTransfer.RestartAt = atoll( pszCmdArg );
				#else
					OneClient.CurrentTransfer.RestartAt = atoi( pszCmdArg );
				#endif
				OneClient.SendReply( "350 REST command successful." );
			} else OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_RETR ) {

			if( !OneClient.CheckPrivileges( CFtpServer::READFILE ) ) {
				OneClient.SendReply( "550 Permission denied." );
				return true;
			}
			if( pszCmdArg ) {

				mypszPath = OneClient.BuildPath( pszCmdArg );
				if( mypszPath && vrt_stat( mypszPath, &st ) == 0 && S_ISREG( st.st_mode ) ) {

					if( OneClient.OpenDataConnection( nCmd ) == false )
						return true;

					OneClient.eStatus = DOWNLOADING;
					strcpy( (OneClient.CurrentTransfer.szPath), mypszPath );
					OneClient.CurrentTransfer.iZlibLevel = OneClient.iZlibLevel;
					OneClient.CurrentTransfer.eDataMode = OneClient.eDataMode;
					pFtpServer->OnClientEventCb( CLIENT_DOWNLOAD, &OneClient, mypszPath );

					RetrieveThread();
				} else
					OneClient.SendReply( "550 File not found." );
			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_STOR || nCmd == CMD_APPE || nCmd == CMD_STOU ) {

			if( !OneClient.CheckPrivileges( CFtpServer::WRITEFILE ) ) {
				OneClient.SendReply( "550 Permission denied." );
				return true;
			}
			if( pszCmdArg || nCmd == CMD_STOU ) {

				char szName[ 32 ];
				if( nCmd == CMD_STOU ) {
					sprintf( szName, "file.%i", rand()%9999999 );
					mypszPath = OneClient.BuildPath( szName );
					if( !mypszPath || !vrt_stat( mypszPath, &st ) ) {
						mypszPath = NULL;
					} else
						OneClient.SendReply2( "150 FILE: %s", szName );
				} else
					mypszPath = OneClient.BuildPath( pszCmdArg );

				if( mypszPath ) {

					if( !OneClient.OpenDataConnection( nCmd ) )
						return true;

					OneClient.eStatus = UPLOADING;
					if( nCmd == CMD_APPE ) {
						if( vrt_stat( mypszPath, &st ) == 0 ) {
							OneClient.CurrentTransfer.RestartAt = st.st_size;
						} else
							OneClient.CurrentTransfer.RestartAt = 0;
					}
					strcpy( OneClient.CurrentTransfer.szPath, mypszPath );
					OneClient.CurrentTransfer.iZlibLevel = OneClient.iZlibLevel;
					OneClient.CurrentTransfer.eDataMode = OneClient.eDataMode;
					pFtpServer->OnClientEventCb( CLIENT_UPLOAD, &OneClient, mypszPath );

					StoreThread();     /* Context pointer for thread main */
				}
			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_SIZE ) {
			if( pszCmdArg ) {
				mypszPath = OneClient.BuildPath( pszCmdArg );
				if( mypszPath && vrt_stat( mypszPath, &st ) == 0 && S_ISREG( st.st_mode ) ) {
					OneClient.SendReply2(
							"213 %li",
						st.st_size );
				} else
					OneClient.SendReply( "550 No such file." );
			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_DELE ) {

			if( !OneClient.CheckPrivileges( CFtpServer::DELETEFILE ) ) {
				OneClient.SendReply( "550 Permission denied." );
				return true;
			}
			if( pszCmdArg ) {
				mypszPath = OneClient.BuildPath( pszCmdArg );
				if( mypszPath && vrt_stat( mypszPath, &st ) == 0 && S_ISREG( st.st_mode ) ) {
					if( vrt_unlink( mypszPath ) != -1 ) {
						OneClient.SendReply( "250 DELE command successful." );
					} else
						OneClient.SendReply( "550 Can' t Remove or Access Error." );
				} else
					OneClient.SendReply( "550 No such file." );
			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_RNFR ) {

			if( !OneClient.CheckPrivileges( CFtpServer::DELETEFILE ) ) {
				OneClient.SendReply( "550 Permission denied." );
				return true;
			}
			if( pszCmdArg ) {
				mypszPath = OneClient.BuildPath( pszCmdArg );
				if( mypszPath && vrt_stat( mypszPath, &st ) == 0 ) {
					strcpy( OneClient.szRenameFromPath, mypszPath );
					OneClient.SendReply( "350 File or directory exists, ready for destination name." );
				} else
					OneClient.SendReply( "550 No such file or directory." );
			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_RNTO ) {

			if( pszCmdArg ) {
				if( OneClient.szRenameFromPath ) {
					mypszPath = OneClient.BuildPath( pszCmdArg );
					dbgprintbtm("Rename %s int %s.\n",OneClient.szRenameFromPath, mypszPath);
					if( mypszPath && vrt_rename( OneClient.szRenameFromPath, mypszPath ) == 0 ) {
						OneClient.SendReply( "250 Rename successful." );
					} else
						OneClient.SendReply( "550 Rename failure." );
					*OneClient.szRenameFromPath = 0;
				} else
					OneClient.SendReply( "503 Bad sequence of commands" );
			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_MKD || nCmd == CMD_XMKD ) {

			if( !OneClient.CheckPrivileges( CFtpServer::CREATEDIR ) ) {
				OneClient.SendReply( "550 Permission denied." );
				return true;
			}
			if( pszCmdArg ) {
				mypszPath = OneClient.BuildPath( pszCmdArg );
				if( mypszPath && vrt_stat( mypszPath, &st ) != 0 ) {
						if( vrt_mkdir( mypszPath, 0777 ) < 0 ) {
						OneClient.SendReply( "550 MKD Error Creating DIR." );
					} else
						OneClient.SendReply( "250 MKD command successful." );
				} else
					OneClient.SendReply( "550 File Already Exists." );
			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else if( nCmd == CMD_RMD || nCmd == CMD_XRMD ) {

			if( !OneClient.CheckPrivileges( CFtpServer::DELETEDIR ) ) {
				OneClient.SendReply( "550 Permission denied." );
				return true;
			}
			if( pszCmdArg ) {
				mypszPath = OneClient.BuildPath( pszCmdArg );
				if( mypszPath && vrt_stat( mypszPath, &st ) == 0 ) {
						if ( vrt_unlink( mypszPath ) < 0  ) {
						OneClient.SendReply( "450 Internal error deleting the directory." );
					} else
						OneClient.SendReply( "250 Directory deleted successfully." );
				} else
					OneClient.SendReply( "550 Directory not found." );
			} else
				OneClient.SendReply( "501 Syntax error in arguments." );
			return true;

		} else {
			OneClient.SendReply( "500 Command not understood." );
			return true;
		}


	return false;
}

void CFtpServer::CClientEntry::LogIn()
{
	bIsLogged = true;
	SendReply( "230 User Logged In." );
	ResetTimeout();
}

void CFtpServer::CClientEntry::LogOut()
{
	bIsLogged = false;
	strcpy( szWorkingDir, "/" );
	if( szRenameFromPath )
		*szRenameFromPath = 0x0;
}

////////////////////////////////////////
// CONTROL CHANNEL
////////////////////////////////////////

bool CFtpServer::CClientEntry::SendReply( const char *pszReply, bool bNoNeedToAlloc /* =0 */ )
{
	if( pszReply ) {
		bool bReturn = false;
		int nLen = strlen( pszReply );
		char *pszBuffer;
		if( !bNoNeedToAlloc ) {
			pszBuffer = new char[ nLen + 2 ];
			if( !pszBuffer ) {
				pFtpServer->OnServerEventCb( MEM_ERROR );
				return false;
			}
			memcpy( pszBuffer, pszReply, nLen );
		} else pszBuffer = (char*) pszReply;
		pFtpServer->OnClientEventCb( SEND_REPLY, this, (void*)pszReply );

		pszBuffer[ nLen  ] = '\r';
		pszBuffer[ nLen + 1 ] = '\n';
		int nbr = m_ControlConnection.Send( pszBuffer, nLen + 2, 0 );

		sgIP_dbgprint("send nbr %s \n",pszBuffer);
		if (nbr >0)
			bReturn = true;
		else
			pFtpServer->OnClientEventCb( CLIENT_SOCK_ERROR, this );
		delete [] pszBuffer;

		return bReturn;
	}
	return false;
}

bool CFtpServer::CClientEntry::SendReply2( const char *pszList, ... )
{
	if( pszList ) {
		char *pszBuffer = new char[ CFTPSERVER_REPLY_MAX_LEN ];
		if( pszBuffer ) {
			va_list args;
			va_start( args, pszList );
			vsnprintf( pszBuffer, CFTPSERVER_REPLY_MAX_LEN - 2, pszList , args);
			return SendReply( pszBuffer, 1 );
		} else
			pFtpServer->OnServerEventCb( MEM_ERROR );

	}
	return false;
}



s32 CFtpServer::CClientEntry::ReceiveLine()
{
sgIP_dbgprint("ReceiveLine\nReceiveLine\n");
	iCmdRecvdLen = 0;
	pszCmdArg = NULL;
	iCmdLen = 0;
	if( psNextCmd && nRemainingCharToParse > 0 )
	{
		memmove( &sCmdBuffer, psNextCmd, nRemainingCharToParse * sizeof( char ) );
		psNextCmd = NULL;
	}

	int iRemainingCmdLen;
	bool bFlushNextLine = false;

	for( ;; ) {

			s32 Err;

sgIP_dbgprint("recv\n");
			Err = m_ControlConnection.ReceiveLine(sCmdBuffer + iCmdRecvdLen , iRemainingCmdLen, 0);
sgIP_dbgprint("wait for %d\n",Err);

		if( Err > 0 )
		{
			nRemainingCharToParse += Err;
		} else if( Err == 0 )
		{
			return RETRY;
		} else
		{
			pFtpServer->OnClientEventCb( CLIENT_SOCK_ERROR, this ) ;
			return ERROR;
		}


		if( nRemainingCharToParse > 0 ) {
			char *s;
			while( nRemainingCharToParse ) {
				--nRemainingCharToParse;
				s = sCmdBuffer + iCmdRecvdLen;
				if( *s == '\r' || *s == '\n' || *s == 0 ) {
					if( iCmdRecvdLen > 0 && !bFlushNextLine ) {
						*s = 0x0;
						iCmdLen = iCmdRecvdLen;
						psNextCmd = s + 1;
						break;
					} else {
						memmove( s, s + 1, nRemainingCharToParse );
						if( bFlushNextLine ) {
							iCmdRecvdLen = 0;
							bFlushNextLine = false;
						}
					}
				} else
					++iCmdRecvdLen;
			}
			if( iCmdLen ) {
				pFtpServer->OnClientEventCb( RECVD_CMD_LINE, this, sCmdBuffer );
				return RECEIVE;
			}
		}

		iRemainingCmdLen = MAX_COMMAND_LINE_LEN - iCmdRecvdLen;
		if( iRemainingCmdLen <= 0 ) {
			if( !bFlushNextLine )
				SendReply( "500 Command line is too long !" );
			iCmdRecvdLen = 0;
			bFlushNextLine = true;
			iRemainingCmdLen = MAX_COMMAND_LINE_LEN;
		}
	}
	return RETRY;
}

int CFtpServer::CClientEntry::ParseLine()
{
	// Separate the Cmd and the Arguments
	char *pszSpace = strchr( sCmdBuffer, ' ' );
	if( pszSpace ) {
		pszSpace[0] = '\0';
		pszCmdArg = pszSpace + 1;
	} else pszCmdArg = NULL;

	// Convert the Cmd to uppercase

		char *psToUpr = sCmdBuffer;
		while(	psToUpr && psToUpr[0] ) {
			*psToUpr = toupper( *psToUpr );
			++psToUpr;
		}

	switch( sCmdBuffer[0] ) {
		case 'A':
			switch( sCmdBuffer[1] ) {
				case 'B':
					if( !strcmp( sCmdBuffer + 2, "OR" ) ) return CMD_ABOR;
					break;
				case 'L':
					if( !strcmp( sCmdBuffer + 2, "LO" ) ) return CMD_ALLO;
					break;
				case 'P':
					if( !strcmp( sCmdBuffer + 2, "PE" ) ) return CMD_APPE;
					break;
			}
			break;
		case 'C':
			switch( sCmdBuffer[1] ) {
				case 'D':
					if( !strcmp( sCmdBuffer + 2, "UP" ) ) return CMD_CDUP;
					break;
				case 'L':
					if( !strcmp( sCmdBuffer + 2, "NT" ) ) return CMD_CLNT;
					break;
				case 'W':
					if( !strcmp( sCmdBuffer + 2, "D" ) ) return CMD_CWD;
					break;
			}
			break;
		case 'D':
			if( !strcmp( sCmdBuffer + 1, "ELE" ) ) return CMD_DELE;
			break;
		case 'E':
			if( !strcmp( sCmdBuffer + 1, "XIT" ) ) return CMD_EXIT;
			break;
		case 'F':
			if( !strcmp( sCmdBuffer + 1, "EAT" ) ) return CMD_FEAT;
			break;
		case 'H':
			if( !strcmp( sCmdBuffer + 1, "ELP" ) ) return CMD_HELP;
			break;
		case 'L':
			if( !strcmp( sCmdBuffer + 1, "IST" ) ) return CMD_LIST;
			break;
		case 'M':
			switch( sCmdBuffer[1] ) {
				case 'D':
					if( !strcmp( sCmdBuffer + 2, "TM" ) ) return CMD_MDTM;
					break;
				case 'K':
					if( !strcmp( sCmdBuffer + 2, "D" ) ) return CMD_MKD;
					break;
				case 'O':
					if( !strcmp( sCmdBuffer + 2, "DE" ) ) return CMD_MODE;
					break;
			}
			break;
		case 'N':
			switch( sCmdBuffer[1] ) {
				case 'L':
					if( !strcmp( sCmdBuffer + 2, "ST" ) ) return CMD_NLST;
					break;
				case 'O':
					if( !strcmp( sCmdBuffer + 2, "OP" ) ) return CMD_NOOP;
					break;
			}
			break;
		case 'O':
			if( !strcmp( sCmdBuffer + 1, "PTS" ) ) return CMD_OPTS;
			break;
		case 'P':
			switch( sCmdBuffer[1] ) {
				case 'A':
					if( sCmdBuffer[2] == 'S' ) {
						switch( sCmdBuffer[3] ) {
							case 'V':
								if( sCmdBuffer[4] == 0 ) return CMD_PASV;
								break;
							case 'S':
								if( sCmdBuffer[4] == 0 ) return CMD_PASS;
								break;
						}
					}
					break;
				case 'O':
					if( !strcmp( sCmdBuffer + 2, "RT" ) ) return CMD_PORT;
					break;
				case 'W':
					if( !strcmp( sCmdBuffer + 2, "D" ) ) return CMD_PWD;
					break;
			}
			break;
		case 'Q':
			if( !strcmp( sCmdBuffer + 1, "UIT" ) ) return CMD_QUIT;
			break;
		case 'R':
			switch( sCmdBuffer[1] ) {
				case 'E':
					switch( sCmdBuffer[2] ) {
						case 'S':
							if( !strcmp( sCmdBuffer + 3, "T" ) ) return CMD_REST;
							break;
						case 'T':
							if( !strcmp( sCmdBuffer + 3, "R" ) ) return CMD_RETR;
							break;
					}
					break;
				case 'M':
					if( !strcmp( sCmdBuffer + 2, "D" ) ) return CMD_RMD;
					break;
				case 'N':
					switch( sCmdBuffer[2] ) {
						case 'F':
							if( !strcmp( sCmdBuffer + 3, "R" ) ) return CMD_RNFR;
							break;
						case 'T':
							if( !strcmp( sCmdBuffer + 3, "O" ) ) return CMD_RNTO;
							break;
					}

					break;
			}
			break;
		case 'S':
			switch( sCmdBuffer[1] ) {
				case 'I':
					switch( sCmdBuffer[2] ) {
						case 'T':
							if( !strcmp( sCmdBuffer + 3, "E" ) ) return CMD_SITE;
							break;
						case 'Z':
							if( !strcmp( sCmdBuffer + 3, "E" ) ) return CMD_SIZE;
							break;
					}
					break;
				case 'T':
					switch( sCmdBuffer[2] ) {
						case 'A':
							if( !strcmp( sCmdBuffer + 3, "T" ) ) return CMD_STAT;
							break;
						case 'O':
							switch( sCmdBuffer[3] ) {
								case 'R':
									if( sCmdBuffer[4] == '\0' ) return CMD_STOR;
									break;
								case 'U':
									if( sCmdBuffer[4] == '\0' ) return CMD_STOU;
									break;
							}
						case 'R':
							if( !strcmp( sCmdBuffer + 3, "U" ) ) return CMD_STRU;
							break;
					}
					break;
				case 'Y':
					if( !strcmp( sCmdBuffer + 2, "ST" ) ) return CMD_SYST;
					break;
			}
			break;

			break;
		case 'T':
			if( !strcmp( sCmdBuffer + 1, "YPE" ) ) return CMD_TYPE;
			break;
		case 'U':
			if( !strcmp( sCmdBuffer + 1, "SER" ) ) return CMD_USER;
			break;
		case 'X':
			switch( sCmdBuffer[1] ) {
				case 'C':
					switch( sCmdBuffer[2] ) {
						case 'U':
							if( !strcmp( sCmdBuffer + 3, "P" ) ) return CMD_XCUP;
							break;
						case 'W':
							if( !strcmp( sCmdBuffer + 3, "D" ) ) return CMD_XCWD;
							break;
					}
					break;
				case 'M':
					if( !strcmp( sCmdBuffer + 2, "KD" ) ) return CMD_XMKD;
				case 'R':
					if( !strcmp( sCmdBuffer + 2, "MD" ) ) return CMD_XRMD;
				case 'P':
					if( !strcmp( sCmdBuffer + 2, "PW" ) ) return CMD_XPWD;
			}
			break;
	}
	return CMD_NONE;
}




////////////////////////////////////////
// DATA CHANNEL
////////////////////////////////////////

bool CFtpServer::CClientEntry::OpenDataConnection(s32 nCmd )
{
	if( eStatus != WAITING )
	{
		SendReply( "425 You're already connected." );
		return false;
	}
	if( eDataConnection == NONE )
	{
		SendReply( "503 Bad sequence of commands." );
		return false;
	}

	if( eDataConnection == PORT )
	{
		SendReply( "150 Opening data channel." );
		OneClient.m_DataConnection.Init(CUnBlockingSocket::DataSocket);
		if (!OneClient.m_DataConnection.Create(SOCK_STREAM))
		   return false;
	}

	if( eDataConnection == PASV ) {
		sgIP_dbgprint("trying to connect on%d \n",ulDataIp);

		if( m_DataConnection.Accept() )
			{
				SendReply( "150 Connection net_accepted." );
				return true;
			}

	} else { // eDataConnection == PORT

			u16 usLen, usStart;
			pFtpServer->GetDataPortRange( &usStart, &usLen );

			CSockAddr csaAddressTemp((long unsigned int) INADDR_ANY, (u32) (usStart + ( rand() % usLen )));
			if( m_DataConnection.Bind(csaAddressTemp)) {

				CSockAddr csaAddress(ulDataIp, usDataPort);
				if (m_DataConnection.Connect(csaAddress)) {
				return true;
				}
			}
	}

	SendReply( "425 Can't open data connection." );
	ResetDataConnection();
	return false;
}
bool CFtpServer::CClientEntry::ResetDataConnection( bool bSyncWait )
{
	eStatus = WAITING;

	{
	m_DataConnection.Cleanup();

	memset( &CurrentTransfer, 0x0, sizeof( CurrentTransfer ) );
	eStatus = WAITING;
	eDataConnection = NONE;
	}
	return true;
}

////////////////////////////////////////
// TRANSFER
////////////////////////////////////////

#ifdef CFTPSERVER_ENABLE_ZLIB
bool CFtpServer::CClientEntry::InitZlib( DataTransfer_t *pTransfer )
{
	int nRet = true;
	pTransfer->zStream.zfree = Z_NULL;
	pTransfer->zStream.zalloc = Z_NULL;
	pTransfer->zStream.opaque = Z_NULL;

	switch( OneClient.eStatus )
	{
		case LISTING:
		case DOWNLOADING:
			nRet = deflateInit( &pTransfer->zStream, pTransfer->iZlibLevel );
			if( nRet == Z_OK )
				return true;
			break;
		case UPLOADING:
			nRet = inflateInit( &pTransfer->zStream );
			if( nRet == Z_OK )
				return true;
			break;
		case WAITING :
			break;
	}

	switch( nRet )
	{
		case Z_MEM_ERROR:
			pFtpServer->OnServerEventCb( MEM_ERROR );
			break;
		case Z_VERSION_ERROR:
			pFtpServer->OnServerEventCb( ZLIB_VERSION_ERROR );
			break;
		case Z_STREAM_ERROR:
			pFtpServer->OnServerEventCb( ZLIB_STREAM_ERROR );
			break;
	}
	return false;
}
#endif

#define size_t lo

bool CFtpServer::CClientEntry::SafeWrite( FILE * hFile, char *pBuffer, u32 nLen )
{
sgIP_dbgprint("wRITE %d\n",nLen );
	u32 wl = 0, k;
	if (nLen ==0) return true;
	while(wl != nLen)
	{
		sgIP_dbgprint("wRITE end wl=%d Len=%d sub=%d\n",wl,nLen,nLen - wl );
		k = fwrite( pBuffer + wl, 1,  nLen - wl, hFile );
		if(k <= 0)
		{
			sgIP_dbgprint("wRITE error %d\n",k );sleep(20);
			 return false;
		}
		else
		{
			sgIP_dbgprint("wRITE end %d\n",k );
		}
		wl += k;
	}
	return true;
}


int CFtpServer::CClientEntry::StoreThread(void)
{
	CFtpServer *pFtpServer = OneClient.pFtpServer;
	struct DataTransfer_t *pTransfer = &OneClient.CurrentTransfer;

	s32 len = 0;
	FILE *hFile = NULL;

	int iflags = O_WRONLY | O_CREAT ;
	OneClient.CurrentTransfer.RestartAt =0;
	if( OneClient.CurrentTransfer.RestartAt > 0 ) {
		iflags |= O_APPEND; //a|b
	} else
		iflags |= O_TRUNC; //w|b

	u32 uiBufferSize = pFtpServer->GetTransferBufferSize();
	char *pBuffer = TransferBufferSize;

	s32 nFlush, nRet;
	char *pOutBuffer = NULL;
	if( pTransfer->eDataMode == ZLIB ) {
		pOutBuffer = TransferBufferZipSize;
		if( !pOutBuffer || !pBuffer ) {
			pFtpServer->OnServerEventCb( MEM_ERROR );
			goto endofstore;
		}
		if( !OneClient.InitZlib( pTransfer ) )
			goto endofstore;
		pTransfer->zStream.next_out = (Bytef*) pOutBuffer;
		pTransfer->zStream.avail_out = uiBufferSize;
	}

	hFile = vrt_fopen(  (char *)pTransfer->szPath, (char *)"wb" );
	sgIP_dbgprint("Open done\n" );
// pTransfer->RestartAt =0; debug
	if( hFile != NULL )
	{
		if( (pTransfer->RestartAt > 0 && fseek( hFile, pTransfer->RestartAt, SEEK_SET) != -1 )
			|| pTransfer->RestartAt == 0 )
		{
			while( 1 ) {

				sgIP_dbgprint("Transfert in progress done\n" );

					len = OneClient.m_DataConnection.Receive( pBuffer, uiBufferSize,0 );

					sgIP_dbgprint("recv end %d\n",len );
					if( len > 0 )
					{
						if( pTransfer->eDataMode == ZLIB ) {
							pTransfer->zStream.avail_in = len;
							pTransfer->zStream.next_in = (Bytef*) pBuffer;
							nFlush = !len ? Z_NO_FLUSH : Z_FINISH;
							do
							{
								nRet = inflate( &pTransfer->zStream, nFlush );
								if( nRet != Z_OK && nRet != Z_STREAM_END && nRet != Z_BUF_ERROR )
									break; // Zlib error
								if( len == 0 && nRet == Z_BUF_ERROR )
									break; // transfer has been interrupt by the client.
								if( pTransfer->zStream.avail_out == 0 || nRet == Z_STREAM_END ) {
									if( !OneClient.SafeWrite( hFile, pOutBuffer, uiBufferSize - pTransfer->zStream.avail_out) )
									{
										len = -1;
										break; // write error
									}
									pTransfer->zStream.next_out = (Bytef*) pOutBuffer;
									pTransfer->zStream.avail_out = uiBufferSize;
								}
							} while( pTransfer->zStream.avail_in != 0 );
							if( nRet == Z_STREAM_END )
								break;
						}
						else
						if( len > 0 )
						{
							if( !OneClient.SafeWrite(hFile, pBuffer, len))
							{
								len = -1;
								break;
							}
						}
						else
						{
							OneClient.SafeWrite( hFile, pBuffer, len);
							break;
						}
					} else
					{
						break; // SOCKET Error
					}
			}
		}
		fclose( hFile );
	}

endofstore:

	if( pTransfer->eDataMode == ZLIB ) {
		deflateEnd( &pTransfer->zStream );
	}
	if( len < 0 || hFile == NULL ) {
		OneClient.SendReply( "550 Can't store file." );
	} else
		OneClient.SendReply( "226 Transfer complete." );

	{
		OneClient.ResetDataConnection( false ); // do not wait sync on self
		if( OneClient.bIsCtrlCanalOpen == true ) {
			OneClient.ResetTimeout();
		} else
			;
	}

	return 0;

}




	int CFtpServer::CClientEntry::RetrieveThread( void )
{
	CFtpServer *pFtpServer = OneClient.pFtpServer;
	struct DataTransfer_t *pTransfer = &OneClient.CurrentTransfer;

	FILE * hFile = NULL;
	s32 BlockSize = 0; s32 len = 0;

	u32 uiBufferSize = pFtpServer->GetTransferBufferSize();
	char *pBuffer = TransferBufferSize;

	int nFlush, nRet;
	char *pOutBuffer = NULL;
	if( pTransfer->eDataMode == ZLIB ) {
		pOutBuffer = TransferBufferZipSize;
		if( !pOutBuffer || !pBuffer ) {
			pFtpServer->OnServerEventCb( MEM_ERROR );
			goto endofretrieve;
		}
		if( !OneClient.InitZlib( pTransfer ) )
			goto endofretrieve;
		pTransfer->zStream.next_in = (Bytef*) pBuffer;
		pTransfer->zStream.next_out = (Bytef*) pOutBuffer;
		pTransfer->zStream.avail_out = uiBufferSize;
	}


	hFile = vrt_fopen( (char *)pTransfer->szPath, (char *)"rb" );
	sgIP_dbgprint("open file to reaf %s \n" ,pTransfer->szPath );
	if( hFile != NULL ) {
	sgIP_dbgprint("file ok %s \n" ,pTransfer->szPath );
		if( pTransfer->RestartAt == 0
			|| ( pTransfer->RestartAt > 0 && fseek(hFile, pTransfer->RestartAt, SEEK_SET ) != -1 ) )
		{
			while((BlockSize = fread(  pBuffer, 1, uiBufferSize, hFile )) > 0) {
	// sgIP_dbgprint("file block %d %s \n",BlockSize ,pTransfer->szPath );

				if( pTransfer->eDataMode == ZLIB ) {
					nFlush = feof( hFile ) ? Z_FINISH : Z_NO_FLUSH;
					pTransfer->zStream.avail_in = BlockSize;
					pTransfer->zStream.next_in = (Bytef*) pBuffer;
					do
					{
						nRet = deflate( &pTransfer->zStream, nFlush );
							sgIP_dbgprint("deflate Code %d \n" ,nRet );
						if( nRet == Z_STREAM_ERROR )
							break;
						if( pTransfer->zStream.avail_out == 0 || nRet == Z_STREAM_END ) {
							len = OneClient.m_DataConnection.Send(pOutBuffer, uiBufferSize - pTransfer->zStream.avail_out, 0 );
							// sgIP_dbgprint("deflate send Ret = %d len %d to Send %d \n" ,nRet,len, uiBufferSize - pTransfer->zStream.avail_out );
							if( len == 0 )
								{ len =-1; break; }
							pTransfer->zStream.next_out = (Bytef*) pOutBuffer;
							pTransfer->zStream.avail_out = uiBufferSize;
						}
					} while( pTransfer->zStream.avail_in != 0 || ( nFlush == Z_FINISH && nRet == Z_OK ) );
					if( len < 0 || nRet == Z_STREAM_ERROR || nFlush == Z_FINISH )
						break;
				} else
				{
					len = OneClient.m_DataConnection.Send( pBuffer, BlockSize,0 );
					if( len == 0 ) break;
				}
			}
		} // else Internal Error
	// sgIP_dbgprint("file block %d %d %s \n",BlockSize , hFile, pTransfer->szPath );

		fclose( hFile );
	}

endofretrieve:

#ifdef CFTPSERVER_ENABLE_ZLIB
	if( pTransfer->eDataMode == ZLIB ) {
		deflateEnd( &pTransfer->zStream );
	}
#endif

	if( len < 0 || hFile == NULL ) {
		OneClient.SendReply( "550 Can't retrieve File." );
	} else
		OneClient.SendReply( "226 Transfer complete." );

	{
		OneClient.ResetDataConnection( false ); // do not wait sync on self
		if( OneClient.bIsCtrlCanalOpen == true ) {
			OneClient.ResetTimeout();
		} else
			;
	}

		return 0;
}

// !!!  psLine must be at least CFTPSERVER_LIST_MAX_LINE_LEN (=MAX_PATH+57) char long.
int CFtpServer::CClientEntry::GetFileListLine( char* psLine, unsigned short mode,long size,	time_t mtime, const char* pszName, bool opt_F )
{

	if( !psLine || !pszName )
		return -1;

	char szYearOrHour[ 6 ] = "";

	static const char *pszMonth[] =
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	static const char szListFile[] = "%c%c%c%c%c%c%c%c%c%c 1 user group %14"
									"l"
									"i %s %2d %s %s%s\r\n";

	struct tm *t = gmtime( (time_t *) &mtime ); // UTC Time
	if( time(NULL) - mtime > 180 * 24 * 60 * 60 ) {
		sprintf( szYearOrHour, "%5d", t->tm_year + 1900 );
	} else
		sprintf( szYearOrHour, "%02d:%02d", t->tm_hour, t->tm_min );

	int iLineLen = sprintf( psLine, szListFile,
		( S_ISDIR( mode ) ) ? 'd' : '-',
		( mode & S_IREAD ) == S_IREAD ? 'r' : '-',
		( mode & S_IWRITE ) == S_IWRITE ? 'w' : '-',
		( mode & S_IEXEC ) == S_IEXEC ? 'x' : '-',
		'-', '-', '-', '-', '-', '-',
		size,
		pszMonth[ t->tm_mon ], t->tm_mday, szYearOrHour,
		pszName,
		( opt_F && S_ISDIR( mode ) ? "/" : "" ) );

	return iLineLen;
}

bool CFtpServer::CClientEntry::AddToListBuffer( DataTransfer_t *pTransfer,
				char *pszListLine, s32 nLineLen,
				char *pBuffer, u32 *nBufferPos, u32 uiBufferSize )
{

	if( pTransfer->eDataMode == ZLIB ) {
		int nRet;
		pTransfer->zStream.avail_in = nLineLen;
		pTransfer->zStream.next_in = (Bytef*) pszListLine;
		int nFlush = pszListLine ? Z_NO_FLUSH : Z_FINISH;
		do
		{
			sgIP_dbgprint("Deflate");
			nRet = deflate( &pTransfer->zStream, nFlush );
			sgIP_dbgprint("End Deflate");
			if( nRet == Z_STREAM_ERROR )
				return false;
			if( pTransfer->zStream.avail_out == 0 || nRet == Z_STREAM_END ) {
				*nBufferPos = uiBufferSize - pTransfer->zStream.avail_out;
				sgIP_dbgprint(" Send");
				if( m_DataConnection.Send( pBuffer, *nBufferPos,0 ) == 0 )
					return false;
				*nBufferPos = 0;
				sgIP_dbgprint("End Send");
				pTransfer->zStream.next_out = (Bytef*) pBuffer;
				pTransfer->zStream.avail_out = uiBufferSize;
			}
		} while( pTransfer->zStream.avail_in != 0 || ( nFlush == Z_FINISH && nRet == Z_OK ) );
		*nBufferPos = uiBufferSize - pTransfer->zStream.avail_out;
	} else
	{
		if( pszListLine ) {
			int nBufferAvailable = uiBufferSize - *nBufferPos;
			int iCanCopyLen = (nLineLen <= nBufferAvailable) ? nLineLen : nBufferAvailable;
			// sgIP_dbgprint("to copy %d",iCanCopyLen);
			memcpy( (pBuffer + *nBufferPos), pszListLine, iCanCopyLen );
			// sgIP_dbgprint("to copy done");
			*nBufferPos += iCanCopyLen;
			if( *nBufferPos == uiBufferSize ) {
				sgIP_dbgprint("send the result sock\n");
				if( m_DataConnection.Send( pBuffer, uiBufferSize,0 ) == 0 )
				{
					sgIP_dbgprint("Max reached send in progress failed\n" );
					return false;
				}
				sgIP_dbgprint("Max reached send in progress ok\n" );
				*nBufferPos = 0;
				if( iCanCopyLen < nLineLen ) {
					// sgIP_dbgprint("to copy next %d",nLineLen - iCanCopyLen);
					memcpy( pBuffer, pszListLine + iCanCopyLen, nLineLen - iCanCopyLen );
					sgIP_dbgprint("to copy next done");
					*nBufferPos = (nLineLen - iCanCopyLen);
				}
			}
		} else { // Flush the buffer.
			if( nBufferPos )
			{
				sgIP_dbgprint("Flush ok\n" );
				int cpt=0;
				if (*nBufferPos>0) {
					cpt = m_DataConnection.Send( pBuffer, *nBufferPos,0);
				}
				sgIP_dbgprint("nbr sent  %d  \n", cpt);
				pFtpServer->OnClientEventCb( SEND_REPLY, this, (void*)"" );
			}
		}
	}
	return true;
}


int CFtpServer::CClientEntry::ListThread( void )

{
	sgIP_dbgprint("ListThread\n" );

	CFtpServer *pFtpServer = OneClient.pFtpServer;
	struct DataTransfer_t *pTransfer = &OneClient.CurrentTransfer;
	struct stat *st = &pTransfer->st;

	int iFileLineLen = 0;
	char *psFileLine = CFTPSERVER_LIST;

	if( psFileLine ) {

		if( pTransfer->opt_d || !S_ISDIR( st->st_mode ) ) {
			sgIP_dbgprint("*DIR MODE \n" );

			char *pszName = strrchr( pTransfer->szPath, '/' );
			iFileLineLen = OneClient.GetFileListLine( psFileLine, st->st_mode, st->st_size,
				st->st_mtime, (( pszName && pszName[ 1 ] ) ? pszName + 1 : "."), pTransfer->opt_F );

			if( iFileLineLen > 0 ) {
				OneClient.m_DataConnection.Send( psFileLine, iFileLineLen, 0);
				//sgIP_dbgprint("*%s\n",psFileLine );
			}
			else
				; sgIP_dbgprint("***nothing\n");

		} else {

			CEnumFile.Init();
			CEnumFileInfo *fi = &CEnumFile;
			u32 uiBufferSize = pFtpServer->GetTransferBufferSize();
			char *pBuffer = TransferBufferSize;
			u32 nBufferPos = 0;

			if( !pBuffer || !fi ) {
				pFtpServer->OnServerEventCb( MEM_ERROR );
				goto endoflist;
			}
			if( pTransfer->eDataMode == ZLIB ) {
				if( !OneClient.InitZlib( pTransfer ) )
					goto endoflist;
				pTransfer->zStream.next_in = (Bytef*) psFileLine;
				pTransfer->zStream.next_out = (Bytef*) pBuffer;
				pTransfer->zStream.avail_out = uiBufferSize;
			}
			sgIP_dbgprint("FIRST FULL PATH ===> %s\n",pTransfer->szPath);

			if( fi->FindFirst( pTransfer->szPath ) ) {
				do
				{
					if( OneClient.CurrentTransfer.nCmd ==  CMD_NLST ) {

						if( fi->pszName[0] != '.' || pTransfer->opt_a ) {
							if( !OneClient.AddToListBuffer( pTransfer, fi->pszName, strlen( fi->pszName ), pBuffer, &nBufferPos, uiBufferSize ) )
								break;
						}

					} else if( fi->pszName[0] != '.' || pTransfer->opt_a ) {

						iFileLineLen = OneClient.GetFileListLine( psFileLine, fi->mode,
							fi->size, fi->mtime, fi->pszName, pTransfer->opt_F );
						// sgIP_dbgprint("***iFileLineLen %d \n",iFileLineLen); // not remove line ???!!!?
						if( !OneClient.AddToListBuffer( pTransfer, psFileLine, iFileLineLen, pBuffer, &nBufferPos, uiBufferSize ) )
							break;

					}

				} while( fi->FindNext(pTransfer->szPath) );
				fi->FindClose();

				// Flush the buffer
				OneClient.AddToListBuffer( pTransfer, NULL, 0, pBuffer, &nBufferPos, uiBufferSize );

			} // else Directory doesn't exist anymore..

endoflist:

#ifdef CFTPSERVER_ENABLE_ZLIB
			if( pTransfer->eDataMode == ZLIB )
				deflateEnd( &pTransfer->zStream );
#endif
		}

	} else
		pFtpServer->OnServerEventCb( MEM_ERROR );

	if( pTransfer->nCmd != CMD_STAT ) {
		OneClient.SendReply( "226 List Transfer complete." );
	} else
		OneClient.SendReply( "213 End of status" );

	{
		OneClient.ResetDataConnection( false ); // do not wait sync on self
		if( OneClient.bIsCtrlCanalOpen == true ) {
			OneClient.ResetTimeout();
		} else
			OneClient.Delete();
	}
	return 0;

}

//////////////////////////////////////////////////////////////////////
// CFtpServer::CEnumFileInfo CLASS
//////////////////////////////////////////////////////////////////////

bool CFtpServer::CEnumFileInfo::FindFirst( const char *pszPath )
{
	if( pszPath ) {
		strcpy( szDirPath, pszPath );
		dp = vrt_diropen( pszPath );
			if( dp != NULL ) {
				if( FindNext(pszPath) )
					return true;
			}
	}
	return false;
}


bool CFtpServer::CEnumFileInfo::FindNext(const char *pszPath)
{
		struct stat st;

		if( vrt_dirnext( dp, dir_entry, &st) == 0)
		{

			int iDirPathLen = strlen( szDirPath );
			int iFileNameLen = strlen( dir_entry );

			if( iDirPathLen + iFileNameLen >= MAX_PATH )
			{
				return false;
			}

			sprintf( szFullPath, "%s%s%s", pszPath,
				( szDirPath[ iDirPathLen - 1 ] == '/' ) ? "" : "/",
				dir_entry );

			if( vrt_stat( szFullPath, &st ) == 0 ) {
				size = st.st_size;
				mode = st.st_mode;
				mtime = st.st_mtime;
			} else {
			sgIP_dbgprint("***ERROR EVALUATE FILE %s\n",szFullPath );
				size = 0;
				mode = 0;
				mtime = 0;
			}

			sprintf( szFullPath, "%s%s%s", szDirPath,
				( szDirPath[ iDirPathLen - 1 ] == '/' ) ? "" : "/",
				dir_entry );
			pszName = szFullPath + strlen( szDirPath ) +
				( ( szDirPath[ iDirPathLen - 1 ] != '/' ) ? 1 : 0 );



			return true;
		}
	return false;
}

bool CFtpServer::CEnumFileInfo::FindClose()
{
		if( dp ) vrt_dirclose( dp );

	return true;
}

extern bool restart_wifi(void);
extern s32 master_stop;

s32 master_stop;
extern void dbgprintsyntesis(const char * text);
extern void debug(const char * text);


void debugwii(const char *format, ...)
{
	char *tmp=0;
	va_list va;
	va_start(va, format);
	if((vasprintf(&tmp, format, va)>=0) && tmp)
	{
		debug(tmp);
		free(tmp);
	}
	va_end(va);
}

void sleep_msec(int msec)
{

		// Sleep( msec );
}



char *
my_inet_ntoa(struct in_addr ina)
{
	static char buf[4*sizeof "123"];
	unsigned char *ucp = (unsigned char *)&ina;

	sprintf(buf, "%d.%d.%d.%d",
		ucp[0] & 0xff,
		ucp[1] & 0xff,
		ucp[2] & 0xff,
		ucp[3] & 0xff);
	return buf;
}

char *
my_ip_ntoa(u32 ip)
{
	static char buf[4*sizeof "123"];
	unsigned char *ucp = (unsigned char *)&ip;

	sprintf(buf, "%d.%d.%d.%d",
		ucp[0] & 0xff,
		ucp[1] & 0xff,
		ucp[2] & 0xff,
		ucp[3] & 0xff);
	return buf;
}


extern void dbgprintsyntesis(const char * text);

void OnServerEvent( s32 Event )
{

	switch( Event )
	{
		case CFtpServer::START_LISTENING:
			dbgprintserver("* Server is listening !\n");
			break;
		case CFtpServer::START_ACCEPTING:
			dbgprintserver("* Server is accepting incoming connexions !\n");
			break;
		case CFtpServer::STOP_LISTENING:
			dbgprintserver("* Server stopped listening !\n");
			break;
		case CFtpServer::STOP_ACCEPTING:
			dbgprintserver("* Server stopped accepting incoming connexions !\n");
			break;
		case CFtpServer::MEM_ERROR:
			dbgprintserver("* Warning, the CFtpServer class could not allocate memory !\n");
			break;
		case CFtpServer::THREAD_ERROR:
			dbgprintserver("* Warning, the CFtpServer class could not create a thread !\n");
			break;
		case CFtpServer::ZLIB_VERSION_ERROR:
			dbgprintserver("* Warning, the Zlib header version differs from the Zlib library version !\n");
			break;
		case CFtpServer::ZLIB_STREAM_ERROR:
			dbgprintserver("* Warning, error during compressing/decompressing data !\n");
			break;
	}

	switch( Event )
	{
		case CFtpServer::START_LISTENING:
			dbgprintsyntesis(tr("Server waits for client"));
			dbgprintclientIP("");
			break;
		case CFtpServer::STOP_LISTENING:
		case CFtpServer::STOP_ACCEPTING:
			dbgprintsyntesis(tr("Server stopped"));
			dbgprintclientIP("");
			break;
		case CFtpServer::MEM_ERROR:
		case CFtpServer::THREAD_ERROR:
		case CFtpServer::ZLIB_VERSION_ERROR:
		case CFtpServer::ZLIB_STREAM_ERROR:
			dbgprintsyntesis(tr("Internal error"));
			break;
	}

}

void OnUserEvent( s32 Event, CFtpServer::CUserEntry *pUser, void *pArg )
{
	switch( Event )
	{
		case CFtpServer::NEW_USER:
			dbgprintserver("* A new user has been created:\n"
					"\tLogin: %s\n" "\tPassword: %s\n" "\tStart directory: %s\n",
				OneUser.GetLogin(), OneUser.GetPassword(), OneUser.GetStartDirectory() );
			break;

		case CFtpServer::DELETE_USER:
			dbgprintserver("* \"%s\"user is being deleted: \n", OneUser.GetLogin() );
			break;
	}
}


void OnClientEvent( s32 Event, CFtpServer::CClientEntry *pClient, void *pArg )
{
	switch( Event )
	{
		case CFtpServer::NEW_CLIENT:
			dbgprintclient_R( tr("Server at: %s")  , my_inet_ntoa( (struct in_addr) *OneClient.GetServerIP() ));
			dbgprintclient_R( tr("Connected with: %s)")  , my_inet_ntoa( (struct in_addr) *OneClient.GetIP() ));

			dbgprintclient_R( tr("A new client has been created:\n"));
			break;

		case CFtpServer::DELETE_CLIENT:
			dbgprintsyntesis(tr("Server started"));
			dbgprintclient_R( tr("A client is being deleted.\n") );
			break;

		case CFtpServer::CLIENT_AUTH:
			dbgprintclient_R( tr("A client has logged-in.\n"));
			break;

		case CFtpServer::CLIENT_DISCONNECT:
			dbgprintclient_R( tr("A client has disconnected.\n") );
			break;

		case CFtpServer::CLIENT_UPLOAD:
			dbgprintclient_R( tr("A client is uploading a file: \"%s\"\n"),
				(char*)pArg );
			break;

		case CFtpServer::CLIENT_DOWNLOAD:
			dbgprintclient_R( tr("A client is downloading a file: \"%s\"\n"),
				(char*)pArg );
			break;

		case CFtpServer::CLIENT_LIST:
			dbgprintclient_R( tr("A client is listing a directory: \"%s\"\n"),
				(char*)pArg );
			break;

		case CFtpServer::CLIENT_CHANGE_DIR:
			dbgprintclient_R( tr("A client has changed its working directory:\n\tFull path: \"%s\"\n\tWorking directory: \"%s\"\n"),
				(char*)pArg, OneClient.GetWorkingDirectory() );
			break;

		case CFtpServer::RECVD_CMD_LINE:
			dbgprintclient_R( tr("Cmd received: %s\n"),(char*) pArg );
			break;

		case CFtpServer::SEND_REPLY:
			dbgprintclient_S( tr("Reply sent: %s\n"),(char*) pArg );
			break;

		case CFtpServer::TOO_MANY_PASS_TRIES:
			dbgprintclient_E( tr("Too many pass tries for (%s)\n"),
				my_inet_ntoa( (struct in_addr)*OneClient.GetIP() ) );
			break;

		case CFtpServer::NO_TRANSFER_TIMEOUT:
			dbgprintclient_E( tr("NO_LOGIN_TIMEOUT\n"));
			break;
		case CFtpServer::CLIENT_SOCK_ERROR:
			dbgprintclient_E( tr("CLIENT_SOCK_ERROR \n"));
			break;
		case CFtpServer::CLIENT_SOFTWARE:
			dbgprintclient_R( tr("CLIENT_SOFTWARE\n"));
			break;
	}

	switch( Event )
	{
		case CFtpServer::NEW_CLIENT:
		case CFtpServer::CLIENT_AUTH:
			dbgprintsyntesis(tr("Connected"));
			dbgprintclientIP(my_inet_ntoa( (struct in_addr) *OneClient.GetIP()));
			break;

		case CFtpServer::DELETE_CLIENT:
		case CFtpServer::CLIENT_DISCONNECT:
			dbgprintsyntesis(tr("Server waits for client"));
			dbgprintclientIP("");
			break;

		case CFtpServer::CLIENT_UPLOAD:
		case CFtpServer::CLIENT_DOWNLOAD:
		case CFtpServer::CLIENT_LIST:
		case CFtpServer::CLIENT_CHANGE_DIR:
		case CFtpServer::RECVD_CMD_LINE:
		case CFtpServer::SEND_REPLY:
			break;
		case CFtpServer::TOO_MANY_PASS_TRIES:
		case CFtpServer::NO_TRANSFER_TIMEOUT:
		case CFtpServer::CLIENT_SOCK_ERROR:
		case CFtpServer::CLIENT_SOFTWARE:
			dbgprintsyntesis(tr("Internal error"));
			break;
	}
}


void Usage() {
	return;
}



void sleep(int aa)
{
for (int a=0; a < aa; a++)
usleep(1000000);
}

enum eServer {
// User events
NOT_INITIALIZED=0,
INITIALIZED,
LISTENING,
ACCEPTING,
CONNECTED,
NO_CONNECTED,
LAUNCHCLIENT,
LAUNCHCLIENTEX,
SHELL
};



u32 launchServer(int status, bool stop)
{
initialise_virtual_path();
return FtpServer.Server("/",  status,  stop) ;
}


extern u32 CFtpServer::Server(const char *drive, int status, bool stop)
{
    static char UserLogin[60];
    static char UserPass[60];
    static char UserStartDirectory [60];
    static unsigned long ulLocalInterface;
    static    char *pszUserLogin = NULL,
    *pszUserPass = NULL;
    if (!stop)
    {
        switch (status)
        {
            case NOT_INITIALIZED:
            {
                ulLocalInterface = INADDR_ANY;
                u32 server = -1;
                bool network_down = true;
                server = -1;


                if (network_down)
                {
                    net_close(server);

                    sgIP_dbgprint("Waiting for network to initialise...");
                    s32 result = -1;
                    while (result < 0) {
                        net_deinit();
                        result = net_init();
                        if (result < 0) sgIP_dbgprint("net_init() failed: [%i] %s, retrying...\n", result, strerror(-result));
                    }
                    if (result >= 0) {
                        u32 ip = 0;
                        do {
                            ip = net_gethostip();
                            if (!ip) sgIP_dbgprint("net_gethostip() failed, retrying...\n");
                            dbgprintWiiIP(my_ip_ntoa(ip));
                        } while (!ip);
                        if (ip) {
                            struct in_addr addr;
                            addr.s_addr = ip;
                            sgIP_dbgprint("Network initialised.  Wii IP address: %s\n", inet_ntoa(addr));
                        }
                    }
                }

                this->Create();


                unsigned char ucUserPriv = CFtpServer::LIST;

                // Default parameters
                this->SetMaxPasswordTries( 3 );
                this->SetNoLoginTimeout( 45 ); // seconds
                this->SetNoTransferTimeout( 90 ); // seconds
                this->SetDataPortRange( 1024, 1050 ); // data TCP-Port range = [100-999]
                this->SetServerCallback(OnServerEvent);
                this->SetUserCallback(OnUserEvent);
                this->SetClientCallback(OnClientEvent);
                pszUserLogin = (char *)"";
                pszUserPass = (char *)"";
                strcpy(pszUserStartDirectory, drive );
                ucUserPriv |= CFtpServer::READFILE;
                this->EnableModeZ( true );
                this->EnableFXP( true );
                ucUserPriv |= CFtpServer::WRITEFILE | CFtpServer::DELETEFILE |CFtpServer::CREATEDIR
				| CFtpServer::DELETEDIR;

                // Load ini parameters

                CsString FilePath("Config.ini");
                CsString Section ("server_prm");
                CsString Entries ("MaxPasswordTries");
                CsString Value ("0");
                CsString Value2 ("0");

                Entries="MaxPasswordTries";
                if (cfgReadIni(&FilePath, &Section, &Entries, &Value)){
                    if (Value.GetLength()>0) this->SetMaxPasswordTries(atoi(Value));
                }
                Entries="NoLoginTimeout";
                if (cfgReadIni(&FilePath, &Section, &Entries, &Value)){
                    if (Value.GetLength()>0) this->SetNoLoginTimeout(atoi(Value));
                }
                Entries="SetNoTransferTimeout";
                if (cfgReadIni(&FilePath, &Section, &Entries, &Value)){
                    if (Value.GetLength()>0) this->SetNoTransferTimeout(atoi(Value));
                }
                Entries="SetDataPortRangeMin";
                if (cfgReadIni(&FilePath, &Section, &Entries, &Value)){
                    if (Value.GetLength()>0) {
                        Entries="SetDataPortRangeMax";
                        if (cfgReadIni(&FilePath, &Section, &Entries, &Value2)){
                            if (Value2.GetLength()>0) this->SetDataPortRange(atoi(Value), atoi(Value2));
                        }
                    }
                }


                this->SetDataPortRange( Settings.FTPServerUser.DataPort, Settings.FTPServerUser.DataPort+10 );


				if(strcmp(Settings.FTPServerUser.UserName, "") != 0) {
                        strcpy(UserLogin,Settings.FTPServerUser.UserName);
                        pszUserLogin=UserLogin;
                }

 				if(strcmp(Settings.FTPServerUser.Password, "") != 0) {
                         strcpy(UserPass,Settings.FTPServerUser.Password);
                        pszUserPass=UserPass;
                }

   				if(strcmp(Settings.FTPServerUser.FTPPath, "") != 0) {
                        strcpy(UserStartDirectory,Settings.FTPServerUser.FTPPath);
                        strcpy(pszUserStartDirectory,Settings.FTPServerUser.FTPPath);
                }

				ucUserPriv=0;
				if (Settings.FTPServerUser.EnableReadFile==on) ucUserPriv |= CFtpServer::READFILE;
				if (Settings.FTPServerUser.EnableListFile==on) ucUserPriv |= CFtpServer::LIST;
				if (Settings.FTPServerUser.EnableWriteFile==on) ucUserPriv |= CFtpServer::WRITEFILE;
				if (Settings.FTPServerUser.EnableDeleteFile==on) ucUserPriv |= CFtpServer::DELETEFILE;
				if (Settings.FTPServerUser.EnableCreateDir==on) ucUserPriv |= CFtpServer::CREATEDIR;
				if (Settings.FTPServerUser.EnableDeleteDir==on) ucUserPriv |= CFtpServer::DELETEDIR;

				if (Settings.FTPServerUser.ZipMode==on) this->EnableModeZ( true );
				else this->EnableModeZ( false );

                if( !pszUserLogin ) {
                    dbgprintserver("/!\\Error: User Login is missing.\n\n" );
                    Usage();
                    return NOT_INITIALIZED;
                }

                UpdateUser( pszUserLogin, pszUserPass, pszUserStartDirectory,drive );

                OneUser.SetPrivileges( ucUserPriv );

                return INITIALIZED;
                break;
            }

            case INITIALIZED :
            {
                if( StartListening( ulLocalInterface, (unsigned short) 21 ) ) {
                    dbgprintserver( "-Server successfuly started ! :)\n" );
                    return ACCEPTING;

                }
                m_apSckControlConnection.Cleanup();
                return INITIALIZED;
            }
			case ACCEPTING :
			{
               if( StartAccepting() )
				return LISTENING ;

				return ACCEPTING;
 			}
            case LISTENING :
            {
                if( RunClient()) {
                    dbgprintserver( "-Server successfuly accepting! :)\n" );
                    return LAUNCHCLIENT;

                }
                else
                {
                    dbgprintserver( "-Unable to accept incoming connections.\n" );
                    this->StopListening();
                    m_apSckControlConnection.Cleanup();
                    return ACCEPTING;
                }
            }
            case LAUNCHCLIENT :
            {
                if ( this->RunClient())
					return LAUNCHCLIENTEX ;
                this->StopListening();
                m_apSckControlConnection.Cleanup();
                return INITIALIZED;
            }
            case LAUNCHCLIENTEX :
            {
                mypszPath=NULL;
                OneClient.RunClientEx();
                return SHELL;
            }
            case SHELL:
            {
                if (OneClient.RunClientShell()==true) return SHELL;

				OneClient.m_ControlConnection.Cleanup();
                OneClient.Delete();

                return NOT_INITIALIZED;

            }
        }
    }
    else
    {
        switch (status)
        {
            case NOT_INITIALIZED:
            {
                break;
            }
            case INITIALIZED :
            case LISTENING :
			case ACCEPTING :
            case LAUNCHCLIENT :
            case LAUNCHCLIENTEX :
            {
                this->StopListening();
                m_apSckControlConnection.Cleanup();
                break;
            }
            case SHELL:
            {
				OneClient.m_ControlConnection.Cleanup();
                OneClient.Delete();
                break;
            }
        }
   }
return NOT_INITIALIZED;
}



