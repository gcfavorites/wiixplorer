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

#include "CFtpServerConfig.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <zlib.h>


#include <sys/stat.h>
#include <sys/dir.h>
#include "UnBlockingSocket\UnBlockingSocket.h"

using namespace nsSocket;


#include <ctype.h>

#include "CFtpServerConfig.h"



extern void sgIP_dbgprint(const char * txt, ...) ;

extern "C" void dbgprintserver(char * txt, ...) ;
extern "C" void dbgprintclient(char * txt, ...) ;
extern "C" void dbgprintclient_R(char * txt, ...) ;
extern "C" void dbgprintclient_E(char * txt, ...) ;
extern "C" void dbgprintclient_S(char * txt, ...) ;
#define MAX_PATH 300
#define SOCKET s32



#ifndef CFTPSERVER_H
#define CFTPSERVER_H

/// Disable warnings for deprecated C-style commands
#define _CRT_SECURE_NO_DEPRECATE



#ifndef INADDR_NONE
#define INADDR_NONE	((u32) 0xffffffff)
#endif
#ifndef INADDR_ANY
#define INADDR_ANY	((u32) 0x00000000)
#endif

 
/**
* @brief CFtpServer class
*
* Runs a full featured ftp server with user control.
* To run and stop the server use StartListening() and StopListening().
*
* Compiler specific settings can be set in CftpServerConfig.h
*/
class CFtpServer
{
public:
	////////////////////////////////////////
	// [CONSTRUCTOR / DESTRUCTOR]
	////////////////////////////////////////

	CFtpServer(){};
	~CFtpServer(){};

	void Create(void);
	void Delete(void);

	bool RunClient( void);
	class CUserEntry;
	class CClientEntry;

	friend class CUserEntry;
	friend class CClientEntry;
	class CEnumFileInfo;
	

private:
	CUnBlockingSocket		m_apSckControlConnection;   ///< socket for connection to ftp-server



public:

	u32 Server(const char *drive, int status, bool stop) ;
	////////////////////////////////////////
	// START / STOP
	////////////////////////////////////////

	/**
	* Ask the Server to Start Listening on the TCP-Port supplied by SetPort().
	*
	* @param  ulAddr  the Network Adress CFtpServer will listen on.
	*		   Example:	INADDR_ANY for all local interfaces and inet_addr( "127.0.0.1" ) for the TCP Loopback interface.
	*
	* @param  usPort  the TCP-Port on which CFtpServer will listen.
	*
	* @return  "true"   on success,
	*          "false"  on error: the supplied Address or TCP-Port may not be valid.
	*/
	bool StartListening( unsigned long ulAddr, u16 usPort );
	bool StartAccepting( void );

	/**
	* Ask the Server to Stop Listening.
	*
	* @return  "true"   on success,
	*          "false"  on error.
	*/
	bool StopListening();

	/**
	* Check if the Server is currently Listening.
	*
	* @return  "true"   if the Server is currently listening
	*          "false"  otherwise.
	*/
	bool IsListening() const { return bIsListening; }

	/**
	* Ask the Server to Start Accepting Clients.
	*
	* @return  "true"   on success,
	*          "false"  on error.
	*/
	/**
	* Check if the Server is currently Accpeting Clients.
	*
	* @return  "true"   if the Server is currently accepting clients
	*          "false"  otherwise.
	*/
	bool IsAccepting() const { return bIsAccepting; }

	////////////////////////////////////////
	// CONFIG
	////////////////////////////////////////

	/**
	* Get the TCP Port on which CFtpServer will listen for incoming clients.
	*
	* @return  the TCP-Port.
	*/
	u16 GetListeningPort() const { return usListeningPort; };

	/**
	* Set the TCP Port Range CFtpServer can use to Send and Receive Files or Data.
	*
	* @param  usStart  the First Port of the Range.
	* @param  uiLen    the Number of Ports, including the First previously given.
	*
	* @return  "true"   on success,
	*          "false"  on error.
	*/
	bool SetDataPortRange( u16 usStart, u16 uiLen );

	/**
	* Get the TCP Port Range CFtpServer can use to Send and Receive Files or Data.
	*
	* @param  usStart  a ointer to the First Port.
	* @param  usLen    a Pointer to the Number of Ports, including the First.
	*
	* @return  "true"   on success,
	*          "false"  on error.
	*/
	bool GetDataPortRange( u16 *usStart, u16 *usLen );

	/**
	* Set the time in which a user has to login.
	*
	* @param  ulSecond  the Timeout delay in Second. Set it to 0 if no timeout.
	*/
	void SetNoLoginTimeout( u32 ulSecond ) { ulNoLoginTimeout = ulSecond; }

	/**
	* Get the no login Timeout.
	*
	* @return  the Timeout delay in Second.
	*/
	u32 GetNoLoginTimeout() const { return ulNoLoginTimeout; }

	/**
	* Set the no transfer ( list, download or upload ) Timeout.
	*
	* @param  ulSecond  the Timeout delay in Seconds. Set it to 0 if no timeout.
	*/
	void SetNoTransferTimeout( u32 ulSecond ) { ulNoTransferTimeout = ulSecond; }

	/**
	* Get the no transfer ( list, download or upload ) Timeout.
	*
	* @return  the Timeout delay in Second.
	*/
	u32 GetNoTransferTimeout() const { return ulNoTransferTimeout; }

	/**
	* Set the delay the Server will wait when checking for the Client's pass.
	*
	* @param  ulMilliSecond  the Timeout delay in Milliseconds. Set it to 0 if no delay.
	*/
	void SetCheckPassDelay( u32 ulMilliSecond ) { uiCheckPassDelay = ulMilliSecond; }

	/**
	* Get the delay the Server will wait when checking for the Client's password.
	*
	* @return  the delay in millisecnds.
	*/
	u32 GetCheckPassDelay() const { return uiCheckPassDelay; }

	/**
	* Set the max allowed password tries per client. After that, the client is disconnected.
	*
	* @param  uiMaxPassTries  the max allowed Password tries.
	*/
	void SetMaxPasswordTries( u32 uiMaxPassTries ) { uiMaxPasswordTries = uiMaxPassTries; }

	/**
	* Get the max allowed password tries per client.
	*
	* @return  the max allowed Password tries per client.
	*/
	u32 GetMaxPasswordTries() const { return uiMaxPasswordTries; }

	/**
	* Enable or disable server-to-server transfer, better known as File eXchange Protocol.
	* Disabling it will result in check when using the FTP PORT command.
	* Enabling it make the server vulnerable to the FTP bounce attack.
	* By default, FXP is disabled.
	*
	* @param  bEnable  true to enable, and false to disable.
	*/
	void EnableFXP( bool bEnable ) { bEnableFXP = bEnable; }

	/**
	* Check if server-to-server transfer is enabled.
	*
	* @return  "true"   if enabled,
	*          "false"  otherwise.
	*/
	bool IsFXPEnabled() const { return bEnableFXP; }

	/**
	* Set the size of the file transfer and directory listing buffer which
	* will be allocated for each client.
	* Default: (32 * 1024)
	* Minimum: (MAX_PATH + 57)
	* Maximum: Operating system dependent
	*
	* @param  uiSize   the transfer buffer size.
	*/
	void SetTransferBufferSize( u32 uiSize ) { uiTransferBufferSize = uiSize; }

	/**
	* Get the size of the file transfer and directory listing buffer which
	* will be allocated for each client.
	* By default, it will be equal to (32 * 1024).
	*
	* @return  the transfer buffer size.
	*/
	u32 GetTransferBufferSize() const { return uiTransferBufferSize; }

	/**
	* Set the size of the file transfer and directory listing socket buffer which
	* will be allocated for each client.
	* Default: (64 * 1024)
	* Minimum: It should be at least equal to the Transfer Buffer Size + 1.
	* Maximum: Operating system dependent
	*
	* @param  uiSize   the transfer socket buffer size.
	*/
	//void SetTransferSocketBufferSize( u32 uiSize ) { uiTransferSocketBufferSize = uiSize; }

	/**
	* Get the size of the file transfer and directory listing socket buffer which
	* will be allocated for each client.
	* By default, it will be equal to (64 * 1024).
	*
	* @return  the transfer socket buffer size.
	*/
	//u32 GetTransferSocketBufferSize() const { return uiTransferSocketBufferSize; }

#ifdef CFTPSERVER_ENABLE_ZLIB
	/**
	* Enable or disable data transfer compresion.
	*
	* @param  bEnable  true to enable, and false to disable.
	*/
	void EnableModeZ( bool bEnable ) { bEnableZlib = bEnable; }

	/**
	* Check if data transfer compression is enabled.
	*
	* @return  "true"   if enabled,
	*          "false"  otherwise.
	*/
	bool IsModeZEnabled() const { return bEnableZlib; }
#endif

	////////////////////////////////////////
	// EVENTS
	////////////////////////////////////////

	/**
	* Enum the events that can be send to the events callbacks.
	*/
	enum eEvents {
		// User events
		NEW_USER,
		DELETE_USER,
		// Client events
		NEW_CLIENT,
		DELETE_CLIENT,
		CLIENT_DISCONNECT,
		CLIENT_AUTH,
		CLIENT_UPLOAD,
		CLIENT_DOWNLOAD,
		CLIENT_LIST,
		CLIENT_CHANGE_DIR,
		RECVD_CMD_LINE,
		SEND_REPLY,
		TOO_MANY_PASS_TRIES,
		NO_LOGIN_TIMEOUT,
		NO_TRANSFER_TIMEOUT,
		CLIENT_SOCK_ERROR,
		CLIENT_SOFTWARE,
		// Server event
		START_LISTENING,
		STOP_LISTENING,
		START_ACCEPTING,
		STOP_ACCEPTING,
		MEM_ERROR,
		THREAD_ERROR,
		ZLIB_VERSION_ERROR,
		ZLIB_STREAM_ERROR
	};

	typedef void (*OnServerEventCallback_t) ( s32 Event );
	typedef void (*OnUserEventCallback_t) ( s32 Event, CFtpServer::CUserEntry *pUser, void *pArg );
	typedef void (*OnClientEventCallback_t) ( s32 Event, CFtpServer::CClientEntry *pClient, void *pArg );

	/**
	* Set the Server event callback.
	*
	* @param  pCallback  the Callback function.
	*/
	void SetServerCallback( OnServerEventCallback_t pCallback )
	{ _OnServerEventCb = pCallback; }

	/**
	* Set the User event callback.
	*
	* @param  pCallback  the Callback function.
	*/
	void SetUserCallback( OnUserEventCallback_t pCallback )
	{ _OnUserEventCb = pCallback; }

	/**
	* Set the Client event callback.
	*
	* @param  pCallback  the Callback function.
	*/
	void SetClientCallback( OnClientEventCallback_t pCallback )
	{ _OnClientEventCb = pCallback; }

	/**
	* Call the Server event callback.
	*
	* @param  Event  the Callback arguments.
	*/
	virtual void OnServerEventCb( s32 Event )
	{ if( _OnServerEventCb ) _OnServerEventCb( Event ); }

	/**
	* Call the User event callback.
	*
	* @param  Event    the Callback arguments.
	* @param  pUser  a pointer to the User class.
	* @param  pArg     a pointer to something that depends on Event.
	*/
	void OnUserEventCb( s32 Event, CFtpServer::CUserEntry *pUser, void *pArg = NULL )
	{ if( _OnUserEventCb ) _OnUserEventCb( Event, pUser, pArg ); }

	/**
	* Call the Client event callback.
	*
	* @param  Event    the Callback arguments.
	* @param  pClient  a pointer to the Client class.
	* @param  pArg     a pointer to something that depends on Event.
	*/
	void OnClientEventCb( s32 Event, CFtpServer::CClientEntry *pClient, void *pArg = NULL )
	{ if( _OnClientEventCb ) _OnClientEventCb( Event, pClient, pArg ); }


	////////////////////////////////////////
	// USER
	////////////////////////////////////////

	/**
	* Enumerate the different Privileges a User can get.
	*/
	enum
	{
		READFILE	= 0x1,
		WRITEFILE	= 0x2,
		DELETEFILE	= 0x4,
		LIST		= 0x8,
		CREATEDIR	= 0x10,
		DELETEDIR	= 0x20
	};
#ifdef CFTPSERVER_ENABLE_EXTRACMD
	enum {
		ExtraCmd_EXEC	= 0x1,
	};
#endif

	/**
	* Create a new User.
	*
	* @param  pszLogin     the User Name.
	* @param  pszPass      the User Password. Can be NULL.
	* @param  pszStartDir  the User Start directory.
	*
	* @return on success  a pointer to the newly created User;
	*         on error    NULL.
	*/
	void UpdateUser(  char *pszLogin,  char *pszPass,  char *pszStartDir,  const char *drive );


private:

	////////////////////////////////////////
	// EVENTS
	////////////////////////////////////////

#ifdef CFTPSERVER_ENABLE_EVENTS
	OnServerEventCallback_t _OnServerEventCb;
	OnUserEventCallback_t _OnUserEventCb;
	OnClientEventCallback_t _OnClientEventCb;
#endif

	////////////////////////////////////////
	// USER
	////////////////////////////////////////

	enum {
		MaxLoginLen = 16,
		MaxPasswordLen = 16,
		MaxRootPathLen = MAX_PATH
	};

	////////////////////////////////////////
	// CLIENT
	////////////////////////////////////////


	/**
	* Add a new Client.
	*
	* @return  on success  a pointer to the new CClientEntry class,
	*          on error    NULL!_
	*/
	void ModifyClient( CUnBlockingSocket *controlSocket );

	struct
	{
		u16 usLen, usStart;
	} DataPortRange;

	bool bIsListening;
	bool bIsAccepting;
	u16 usListeningPort;
	
	char pszUserStartDirectory[MAX_PATH];


	////////////////////////////////////////
	// FILE
	////////////////////////////////////////

	/**
	* Simplify a Path.
	*/
	static bool SimplifyPath( char *pszPath );


	////////////////////////////////////////
	// CONFIG
	////////////////////////////////////////

	u32 uiMaxPasswordTries;
	u32 uiCheckPassDelay;
	u32 ulNoTransferTimeout, ulNoLoginTimeout;
	u32 uiTransferBufferSize ; //, uiTransferSocketBufferSize;

	bool bEnableZlib;
	bool bEnableFXP;

#endif

};

//////////////////////////////////////////////////////////////////////
// CFtpServer::CUserEntry CLASS
//////////////////////////////////////////////////////////////////////
/**
* @brief CFtpServer::CUserEntry class
*
* One instance of this class will be allocated for each user.
*/
class CFtpServer::CUserEntry
{
public:

	CUserEntry() {};
	~CUserEntry() {};

	friend class CFtpServer;
	friend class CFtpServer::CClientEntry;

	/**
	* Set the Privileges of a User.
	*
	* @param  ucPriv  the user's privileges separated by the bitwise inclusive binary operator "|".
	*
	* @return  "true"   on success,
	*          "false"  on error.
	*/
	void Init();

	/**
	* Set the Privileges of a User.
	*
	* @param  ucPriv  the user's privileges separated by the bitwise inclusive binary operator "|".
	*
	* @return  "true"   on success,
	*          "false"  on error.
	*/
	bool SetPrivileges( unsigned char ucPriv );

	/**
	* Get a User's privileges
	*
	* @return  The user's privileges concatenated with the bitwise inclusive binary operator "|".
	*/
	unsigned char GetPrivileges() const { return ucPrivileges; }
	/**
	* Get a pointer to the User's Name.
	*
	* @return  A pointer to the User's Name.
	*/
	const char *GetLogin() const { return szLogin; }

	/**
	* Get a pointer to the User's Password.
	*
	* @return	A pointer to the User's Password.
	*/
	const char *GetPassword() const { return szPassword; }

	/**
	* Get a pointer to the User's Start Directory.
	*
	* @return  A pointer to the User's Start Directory.
	*/
	const char *GetStartDirectory() const { return szStartDirectory; }

#ifdef CFTPSERVER_ENABLE_EXTRACMD
	/**
	* Set the supported Extra-Commands of a User.
	*
	* @param  dExtraCmd  the user's Extra-Commands concatenated with the bitwise inclusive binary operator "|".
	*
	* @return  "true"   on success,
	*          "false"  on error.
	*/
	bool SetExtraCommand( unsigned char dExtraCmd );

	/**
	* Get the supported Extra-Commands of a User.
	*
	* @return  The user's Extra-Commands concatenated with the bitwise inclusive binary operator "|".
	*/
	unsigned char GetExtraCommand() const { return ucExtraCommand; }

#endif

private:


#ifdef CFTPSERVER_ENABLE_EXTRACMD
	unsigned char ucExtraCommand;
#endif
	bool bDelete;
	bool bIsEnabled;
	unsigned char ucPrivileges;
	char szLogin[ MaxLoginLen + 1 ];
	char szPassword[ MaxPasswordLen + 1 ];
	char szStartDirectory[ MaxRootPathLen + 1 ];
	class CFtpServer *pFtpServer;
} OneUser;




//////////////////////////////////////////////////////////////////////
// CFtpServer::CClientEntry CLASS
//////////////////////////////////////////////////////////////////////
/**
* @brief CFtpServer::CClientEntry class
*
* One instance of this class will be allocated for each client.
*/
class CFtpServer::CClientEntry
{



public:

CClientEntry(){};
~CClientEntry(){};

	friend class CFtpServer;
	
	
	/**
	* Get a pointer to a in_addr structure representing the Client's IP.
	*
	* @return  A pointer to a in_addr structure.
	*/
	struct in_addr *GetIP() const { return (struct in_addr*)&ulClientIP; }

	/**
	* Get a pointer to a in_addr structure representing the Server's IP the client is connected to.
	*
	* @return  A pointer to a in_addr structure.
	*/
	struct in_addr *GetServerIP() const { return (struct in_addr*)&ulServerIP; }

	/**
	* Check if the client is logged-in.
	*
	* @return  "true"   if the client is logged in,
	*          "false"  otherwise.
	*/
	bool IsLogged() const { return bIsLogged; }

	/**
	* Check that the client has a or several privileges.
	*
	* @param  ucPriv  One or serveral privileges to check.
	*
	* @return  "true"   if the client has all the supplied privileges,
	*          "false"  otherwise.
	*/
	bool CheckPrivileges( unsigned char ucPriv ) const;

	/**
	* Get the CWD ( Current Working Directory ) of the Client.
	*/
	char *GetWorkingDirectory() { return szWorkingDir; }

	/**
	* Kick the Client. This is a non-blocking function.
	*
	* @return  "true"   on success,
	*          "false"  on error.
	*/
	bool InitDelete();

	void Delete();

	bool Init( void);
	unsigned short usDataPort;
	

	void RunClientEx(void);
	u32 RunClientShell(void);
private:

	////////////////////////////////////////
	// SHELL
	////////////////////////////////////////

	bool bIsLogged;

	void LogIn();
	void LogOut();


	CUnBlockingSocket		m_ControlConnection;   ///< socket for connection to ftp-server

	void ResetTimeout() { time( &tTimeoutTime ); }

	/// Enum the different Status a Client can have.
	enum Status_t {
		WAITING = 1,
		LISTING,
		UPLOADING,
		DOWNLOADING
	} volatile eStatus;

	unsigned long ulDataIp;

	char szWorkingDir[ MAX_PATH + 3 + 1 ];
	char szRenameFromPath[ MAX_PATH + 1 ];

	
	time_t tTimeoutTime;

	enum {
		MAX_COMMAND_LINE_LEN = MAX_PATH + 32
	};
	char sCmdBuffer[ MAX_COMMAND_LINE_LEN + 1 ];
	char *pszCmdArg;
	char *psNextCmd;
	s32 iCmdLen;
	s32 iCmdRecvdLen;
	s32 nRemainingCharToParse;

	s32 ParseLine();
	bool ReceiveLine();

	u32 nPasswordTries;

	static s32 Shell( void);
	enum
	{
		CMD_NONE = -1,
		CMD_QUIT,
		CMD_USER,
		CMD_PASS,
		CMD_NOOP,
		CMD_ALLO,
		CMD_SITE,
		CMD_HELP,
		CMD_SYST,
		CMD_STRU,
		CMD_MODE,
		CMD_TYPE,
		CMD_CLNT,
		CMD_PORT,
		CMD_PASV,
		CMD_LIST,
		CMD_NLST,
		CMD_CWD,
		CMD_XCWD,
		CMD_FEAT,
		CMD_MDTM,
		CMD_PWD,
		CMD_XPWD,
		CMD_CDUP,
		CMD_XCUP,
		CMD_STAT,
		CMD_ABOR,
		CMD_REST,
		CMD_RETR,
		CMD_STOR,
		CMD_APPE,
		CMD_STOU,
		CMD_SIZE,
		CMD_DELE,
		CMD_RNFR,
		CMD_RNTO,
		CMD_MKD,
		CMD_XMKD,
		CMD_RMD,
		CMD_XRMD,
		CMD_OPTS,
		CMD_EXIT = 100
	};

	////////////////////////////////////////
	// TRANSFER
	////////////////////////////////////////


	s32 iZlibLevel;

	/// Enum the differents Modes of Transfer.
	enum DataMode_t {
		STREAM,
		ZLIB
	} eDataMode;

	/// Enum the differents Type of Transfer.
	enum DataType_t {
		ASCII,
		BINARY, ///< equals to IMAGE
		EBCDIC
	} eDataType;

	/// Enum the differents Modes of Connection for transfering Data.
	enum DataConnection_t {
		NONE,
		PASV,
		PORT
	} eDataConnection;

	CUnBlockingSocket m_DataConnection;  

	struct DataTransfer_t
	{
		CFtpServer::CClientEntry *pClient;
		s32 RestartAt;
		struct stat st;

		s32 nCmd;
		bool opt_a, opt_d, opt_F, opt_l;

		s32 iZlibLevel;
		z_stream zStream;

		DataMode_t eDataMode;
		DataType_t eDataType;

		char szPath[ MAX_PATH + 1 ];
		// pthread_t TransferThreadID;

	} CurrentTransfer;


	bool InitZlib( DataTransfer_t *pTransfer );


	bool OpenDataConnection( s32 nCmd ); ///< Open the Data Channel in order to transmit data.
	bool ResetDataConnection( bool bSyncWait = true ); ///< Close the Data Channel.

	static s32 RetrieveThread(void);
	static s32 StoreThread(void);
	static s32 ListThread(void);
	
	char localClientPath[MAX_PATH];

	////////////////////////////////////////
	// FILE
	////////////////////////////////////////

	bool SafeWrite( FILE *hFile, char *pBuffer, s32 nLen );

	/**
	* Build a Full-Path using:
	*
	* @li  the Client's User Start Directory,
	* @li  the Client's Working Directory,
	* @li  the Client Command.
	*
	* @note  Call interally BuildVirtualPath() and store a pointer to the virtual path in pszVirtualPath.
	*/
	char* BuildPath( char* szAskedPath, char **pszVirtualPath = NULL );

	/**
	* Build a Virtual-Path using:
	*
	* @li  the Client's Working Directory,
	* @li  the Client Command.
	*/
	char* BuildVirtualPath( char* szAskedPath );

	/**
	* Build a list line.
	*
	* @warning  psLine must be at least CFTPSERVER_LIST_MAX_LINE_LEN chars long.
	*/
	s32 GetFileListLine( char* psLine, unsigned short mode,

		long size,
		time_t mtime, const char* pszName, bool opt_F );

	/**
	* Copy the list line to a buffer, and send it to the client when full.
	*/
	bool AddToListBuffer( DataTransfer_t *pTransfer,
		char *pszListLine, s32 nLineLen,
		char *pBuffer, u32 *nBufferPos, u32 uiBufferSize );

	////////////////////////////////////////
	// OTHER
	////////////////////////////////////////

	unsigned long ulServerIP;
	unsigned long ulClientIP;

	bool bIsCtrlCanalOpen;

	class CFtpServer *pFtpServer;
	
	/**
	* Send a reply to the Client.
	*
	* @return  "true"   on success,
	*          "false"  on error: The Socket may be invalid or the Connection may have been interupted.
	*/
	bool SendReply( const char *pszReply, bool bNoNeedToAlloc = 0 );

	/**
	* Send a custom reply to the Client.
	*
	* @return  "true"   on success;
	*          "false"  on error: The Socket may be invalid or the Connection may have been interupted.
	*
	* @note  Call interally SendReply().
	*/
	bool SendReply2( const char *pszList, ... );
} OneClient ;




//////////////////////////////////////////////////////////////////////
// CFtpServer::CEnumFileInfo CLASS
//////////////////////////////////////////////////////////////////////

/**
* Layer of abstraction used to list file on several Operating Systems.
*/


class CFtpServer::CEnumFileInfo
{
public:
	CEnumFileInfo() { memset( this, 0x0, sizeof( CEnumFileInfo ) ); }
	bool Init() { memset( this, 0x0, sizeof( CEnumFileInfo ) ); return true; }
	bool FindFirst( const char *pszPath );
	bool FindNext(const char *pszPath);
	bool FindClose();

	DIR_ITER* dp;
	char dir_entry[256]; // may need to be 256 instead of a constant
	s32 kk;

	FILE *hFile;
	char pszTempPath[ MAX_PATH + 1 ];
	char *pszName;
	char szDirPath[ MAX_PATH + 1 ];
	char szFullPath[ MAX_PATH + 1 ];
	s32 size;
	time_t mtime; // similar to st_mtime.
	u16 mode; // similar to st_mode.
};



