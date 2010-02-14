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

#include <limits>
#include <algorithm>
#include <string.h>
#include "FTPClientconfig.h"
#include "FTPClientListParse.h"
#include "FTPClientFileState.h"


#include "FTPclient.h"

extern "C" void waitpersecond ( unsigned int nbr );

using namespace nsFTP;

#define MAX_TransferBufferSize 32 * 1024
extern char TransferBufferSize[MAX_TransferBufferSize];
#define MAX_TransferBufferSize 32 * 1024
extern char TransferBufferZipSize[];


extern void waitkey(void );


using namespace nsHelper;

namespace nsFTP
{
	class CMakeString
	{
	public: 
		CMakeString& operator<<(DWORD dwNum)
		{
			DWORD dwTemp = dwNum;
			int iCnt=1; // name lookup of 'iCnt' changed for new ISO 'for' scoping
			for( ; (dwTemp/=10) != 0; iCnt++ )
				;

			m_str.resize(m_str.size() + iCnt);
			tsprintf(&(*m_str.begin()), _T("%s%u"), m_str.c_str(), dwNum);

			return *this;
		}

		CMakeString& operator<<(const tstring& strAdd)
		{
			m_str += strAdd;
			return *this;
		}

		operator tstring() { return m_str; }

	private:
		tstring m_str;
	};

	tstring& ReplaceStr(tstring& strTarget, const tstring& strToReplace, const tstring& strReplacement)
	{
		size_t pos = strTarget.find(strToReplace);
		while( pos != tstring::npos )
		{
			strTarget.replace(pos, strToReplace.length(), strReplacement);
			pos = strTarget.find(strToReplace, pos+1);
		}
		return strTarget;
	}

// File
	CFile::CFile() : m_pFile(NULL) {}

	CFile::~CFile()
	{
		Close();
	}

	bool CFile::Open(const tstring* strFileName, const tstring& strMode)
	{
		m_pFile = fopen(strFileName->c_str(), strMode.c_str());
		return m_pFile!=NULL;
	}

	bool CFile::Close()
	{
		FILE* pFile = m_pFile;
		m_pFile = NULL;
		return pFile && fclose(pFile)==0;
	}

	bool CFile::Seek(u64 lOffset, T_enOrigin enOrigin)
	{
		return m_pFile && fseek(m_pFile, lOffset, enOrigin)==0;
	}

	long CFile::Feof() {
		if( !m_pFile )
			return -1L;
		return feof( m_pFile );
	}


	long CFile::Tell()
	{
		if( !m_pFile )
			return -1L;
		return ftell(m_pFile);
	}

	size_t CFile::Write(const void* pBuffer, size_t itemSize, size_t itemCount)
	{
		if( !m_pFile )
			return 0;
		return  fwrite(pBuffer, itemSize, itemCount, m_pFile); // itemSize * itemCount; /// ESSAI PAPA
	}

	size_t CFile::Read(void* pBuffer, size_t itemSize, size_t itemCount)
	{

		if( !m_pFile )
			return 0;

		return fread(pBuffer, itemSize, itemCount, m_pFile);
	}

	void CFile::OnBytesReceived(const char * vBuffer, long lReceivedBytes)
	{
		int ret = Write(vBuffer, sizeof(TByteVector::value_type), lReceivedBytes);
	}

	void CFile::OnPreBytesSend(char * vBuffer, unsigned int vBufferSize, size_t& bytesToSend)
	{
		bytesToSend = Read(vBuffer, sizeof(TByteVector::value_type), vBufferSize);
	}

	void COutputStream::SetStartPosition()
	{
		m_nStart =0;
	}


// Mem file
	CMemFile::CMemFile() 
	{
		// Close();
	}

	CMemFile::~CMemFile()
	{
		Close();
	}

	bool CMemFile::Open(const void *ptr, u64 len)
	{
	m_pFile = (void *)ptr;
	m_len = len;
	m_pos = 0;
	m_final_pos=0;
	}

	bool CMemFile::Close()
	{
	m_pFile = 0;
	m_len = 0;
	m_pos = 0;
	}

	bool CMemFile::Seek(u64 lOffset, T_enOrigin enOrigin)
	{
	boolean cr = true;
 
	if( !m_pFile )
		return false;
			
	switch (enOrigin) 
	{ 
	case SEEK_SET: // from beginning
		if ( lOffset > m_len ) { m_pos = m_len; cr = false ; } 
		else m_pos = lOffset;
		break;
	case SEEK_CUR: // from corr pos
		if ( m_pos + lOffset > m_len ) { m_pos = m_len; cr = false ; } 
		else m_pos = m_pos + lOffset;
		break;
	case SEEK_END: // from end of file
		if ( lOffset > m_pos) { m_pos = m_len - lOffset; cr = false ; } 
		else m_pos = m_len - lOffset;
		break;
	}
	return cr;
	}

	u64 CMemFile::Feof() {
	if( !m_pFile )
		return -1L;
	if (m_pos == m_len) return 0;
	return -1L;
	}


	u64 CMemFile::Tell()
	{
		if( !m_pFile )
			return -1L;
		return m_pos;
	}

	size_t CMemFile::Write(const void* pBuffer, size_t itemSize, size_t itemCount)
	{
		if( !m_pFile )
			return 0;
		if (itemSize*itemCount + m_pos > m_len) return 0; 
		memcpy(m_pFile + m_pos, pBuffer, itemSize*itemCount); 
		m_pos = m_pos + (itemSize*itemCount);
		
		if (m_final_pos <  m_pos) m_final_pos = m_pos;

		return itemSize*itemCount;
	}

	size_t CMemFile::Read(void* pBuffer, size_t itemSize, size_t itemCount)
	{
		u32 mx_read = itemSize*itemCount; 
		if( !m_pFile )
			return 0;

		if (itemSize*itemCount + m_pos > m_len) mx_read = m_len - m_pos;  
		memcpy(pBuffer, m_pFile + m_pos, mx_read);
		m_pos = mx_read + m_pos;

		m_pos = m_pos + (itemSize*itemCount);

		return mx_read;
	}

	void CMemFile::OnBytesReceived(const char * vBuffer, long lReceivedBytes)
	{
		int ret = Write(vBuffer, sizeof(TByteVector::value_type), lReceivedBytes);
	}

	void CMemFile::OnPreBytesSend(char * vBuffer, unsigned int vBufferSize, size_t& bytesToSend)
	{
		bytesToSend = Read(vBuffer, sizeof(TByteVector::value_type), vBufferSize);
	}

	bool COutputStream::GetNextLine(tstring& strLine)// const
	{
		char * lpsz;
		char *m_pText;
		unsigned long m_CurrentPos=0;

		m_pText =  m_vBufferStream;
		if(m_vBufferLength <= m_nStart ) 
			return false;

		lpsz = strstr(m_pText + m_nStart, mc_strEolCharacterSequence.c_str() );

		if (lpsz == NULL) return false ;
		else
			m_CurrentPos = (int)(lpsz - m_pText);

		strLine.assign(m_pText + m_nStart, m_CurrentPos-m_nStart);
		m_nStart = m_CurrentPos + mc_strEolCharacterSequence.length();
		return true;
	}

	void COutputStream::OnBytesReceived(char * vBuffer, long lReceivedBytes)
	{
		m_vBufferStream = vBuffer;
		m_vBufferLength = lReceivedBytes;
		//sgIP_dbgprint("Recived %s", m_vBufferStream);
	}

};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/// constructor
/// @param[in] pSocket Instance of socket class which will be used for 
///                    communication with the ftp server.
///                    CFTPClient class takes ownership of this instance.
///                    It will be deleted on destruction of this object.
///                    If this pointer is NULL, the CUnBlockingSocket implementation
///                    will be used. 
///                    This gives the ability to set an other socket class.
///                    For example a socket class can be implemented which simulates
///                    a ftp server (for unit testing).
/// @param[in] uiTimeout Timeout used for socket operation.
/// @param[in] uiBufferSize Size of the buffer used for sending and receiving
///                         data via sockets. The size have an influence on 
///                         the performance. Through empiric test i come to the
///                         conclusion that 2048 is a good size.
/// @param[in] uiResponseWait Sleep time between receive calls to socket when getting 
///                           the response. Sometimes the socket hangs if no wait time
///                           is set. Normally not wait time is necessary.
CFTPClient::CFTPClient(unsigned int uiTimeout/*=10*/, 
					   unsigned int uiBufferSize/*=2048*/, unsigned int uiResponseWait/*=0*/) :
mc_uiTimeout(uiTimeout),
mc_uiResponseWait(uiResponseWait),
mc_strEolCharacterSequence(_T("\r\n")),
m_fTransferInProgress(false),
m_fAbortTransfer(false),
m_fResumeIfPossible(true),
m_coutputstream(mc_strEolCharacterSequence),
m_apCurrentRepresentation(CType::ASCII()),
iZlibLevel(8),
numberofFiles(0),
m_vBufferSize(MAX_TransferBufferSize),
mCFTPFileStatusNbr(200),
m_vBuffer(TransferBufferSize)
{

}


/// Enables or disables resuming for file transfer operations.
/// @param[in] fEnable True enables resuming, false disables it.
void CFTPClient::SetResumeMode(bool fEnable/*=true*/)
{
	m_fResumeIfPossible=fEnable;
}

/// Opens the control channel to the FTP server.
/// @param[in] strServerHost IP-address or name of the server
/// @param[in] iServerPort Port for channel. Usually this is port 21.
bool CFTPClient::OpenControlChannel(const tstring* strServerHost, unsigned int ushServerPort/*=DEFAULT_FTP_PORT*/)
{
	CloseControlChannel();

	bool cr = m_apSckControlConnection.Create(SOCK_STREAM);

	if (cr==false) {
		return cr;
		}

	CSockAddr adr = m_apSckControlConnection.GetHostByName(strServerHost->c_str(), ushServerPort);

	cr = m_apSckControlConnection.Connect(adr);


	return cr;
}

/// Returns the connection state of the client.
bool CFTPClient::Init()
{
s32 server;

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
				} while (!ip);
				if (ip) {
					struct in_addr addr;
					addr.s_addr = ip;
					sgIP_dbgprint("Network initialised.  Wii IP address: %s\n", inet_ntoa(addr));
				}
			}
		}

	m_apSckControlConnection.Init(CUnBlockingSocket::ApplicationSocket);
	m_sckDataConnection.Init(CUnBlockingSocket::DataSocket);          /// Socket data connection

	return true;
}


/// Returns the connection state of the client.
bool CFTPClient::IsConnected()
{
	return m_apSckControlConnection.Isconnected();
}

/// Returns true if a download/upload is running, otherwise false.
bool CFTPClient::IsTransferringData()
{
	return m_fTransferInProgress;
}

/// Closes the control channel to the FTP server.
void CFTPClient::CloseControlChannel() 
{
	if (m_apSckControlConnection.Close()) {
		// m_apCurrentRepresentation=0; // ADE ......
		return;
	}
	m_apSckControlConnection.Cleanup();

}

/// Analyse the repy code of a ftp-server-response.
/// @param[in] Reply Reply of a ftp server.
/// @retval FTP_OK    All runs perfect.
/// @retval FTP_ERROR Something went wrong. An other response was expected.
/// @retval NOT_OK    The command was not accepted.
int CFTPClient::SimpleErrorCheck(CReply& Reply)
{
	if( Reply.Code().IsNegativeReply() )
		return FTP_NOTOK;
	else if( Reply.Code().IsPositiveCompletionReply() )
		return FTP_OK;

	ASSERT( Reply.Code().IsPositiveReply() );

	return FTP_ERROR;
}
enum {LO=-2,      ///< Logged On
ER=-1,      ///< Error
NUMLOGIN=9, ///< currently supports 9 different login sequences
};

const int iLogonSeq[NUMLOGIN][18] = {
	// this array stores all of the logon sequences for the various firewalls 
	// in blocks of 3 nums.
	// 1st num is command to send, 
	// 2nd num is next point in logon sequence array if 200 series response  
	//         is rec'd from server as the result of the command,
	// 3rd num is next point in logon sequence if 300 series rec'd
	{ 0,LO,3,    1,LO, 6,   2,LO,ER                                  }, // no firewall
	{ 3, 6,3,    4, 6,ER,   5,ER, 9,   0,LO,12,   1,LO,15,   2,LO,ER }, // SITE hostname
	{ 3, 6,3,    4, 6,ER,   6,LO, 9,   1,LO,12,   2,LO,ER            }, // USER after logon
	{ 7, 3,3,    0,LO, 6,   1,LO, 9,   2,LO,ER                       }, // proxy OPEN
	{ 3, 6,3,    4, 6,ER,   0,LO, 9,   1,LO,12,   2,LO,ER            }, // Transparent
	{ 6,LO,3,    1,LO, 6,   2,LO,ER                                  }, // USER with no logon
	{ 8, 6,3,    4, 6,ER,   0,LO, 9,   1,LO,12,   2,LO,ER            }, // USER fireID@remotehost
	{ 9,ER,3,    1,LO, 6,   2,LO,ER                                  }, // USER remoteID@remotehost fireID
	{10,LO,3,   11,LO, 6,   2,LO,ER                                  }  // USER remoteID@fireID@remotehost
};


/// Logs on to an ftp-server.
/// @param[in] logonInfo Structure with logon information.
bool CFTPClient::Login(const CLogonInfo& logonInfo)
{
	CReply Reply;
	m_LastLogonInfo = logonInfo;

	// are we connecting directly to the host (logon type 0) or via a firewall? (logon type>0)
	tstring   strTemp;
	unsigned int    ushPort=0;

	m_feat = logonInfo.Feat();

	if( logonInfo.FwType() == CFirewallType::None())
	{
		strTemp = logonInfo.Hostname();
		ushPort = logonInfo.Hostport();
	}
	else
	{
		strTemp = logonInfo.FwHost();
		ushPort = logonInfo.FwPort();
	}

	tstring strHostnamePort(logonInfo.Hostname());
	if( logonInfo.Hostport()!=DEFAULT_FTP_PORT )
		strHostnamePort = CMakeString() << logonInfo.Hostname() << _T(":") << logonInfo.Hostport(); // add port to hostname (only if port is not 21)

	if( IsConnected() )
		Logout();

	if( !OpenControlChannel(&strTemp, ushPort) )
		return false;

	// get initial connect msg off server

	if( !GetResponse(Reply) || !Reply.Code().IsPositiveCompletionReply() )
		return false;

	int iLogonPoint=0;

	// go through appropriate logon procedure
	while( true )
	{
		switch(iLogonSeq[logonInfo.FwType().AsEnum()][iLogonPoint])
		{
		case 0:
			strTemp=_T("USER ") + logonInfo.Username();
			break;
		case 1:
			strTemp=_T("PASS ") + logonInfo.Password();
			break;
		case 2:
			strTemp=_T("ACCT ") + logonInfo.Account();
			break;
		case 3:
			strTemp=_T("USER ") + logonInfo.FwUsername();
			break;
		case 4:
			strTemp=_T("PASS ") + logonInfo.FwPassword();
			break;
		case 5:
			strTemp=_T("SITE ") + strHostnamePort;
			break;
		case 6:
			strTemp=_T("USER ") + logonInfo.Username() + _T("@") + strHostnamePort;
			break;
		case 7:
			strTemp=_T("OPEN ") + strHostnamePort;
			break;
		case 8:
			strTemp=_T("USER ") + logonInfo.FwUsername() + _T("@") + strHostnamePort;
			break;
		case 9:
			strTemp=_T("USER ") + logonInfo.Username() + _T("@") + strHostnamePort + _T(" ") + logonInfo.FwUsername();
			break;
		case 10:
			strTemp=_T("USER ") + logonInfo.Username() + _T("@") + logonInfo.FwUsername() + _T("@") + strHostnamePort;
			break;
		case 11:
			strTemp=_T("PASS ") + logonInfo.Password() + _T("@") + logonInfo.FwPassword();
			break;
		}

		// send command, get response

		if( !SendCommand(strTemp, Reply) )
			return false;

		if( !Reply.Code().IsPositiveCompletionReply() && !Reply.Code().IsPositiveIntermediateReply() )
			return false;

		const unsigned int uiFirstDigitOfReplyCode = CCnv::TStringToLong(Reply.Code().Value())/100;
		iLogonPoint=iLogonSeq[logonInfo.FwType().AsEnum()][iLogonPoint + uiFirstDigitOfReplyCode-1]; //get next command from array
		switch(iLogonPoint)
		{
		case ER: // ER means somewhat has gone wrong
			{  
				ReportError(CCnv::ConvertToTString(__FILE__), __LINE__);
		 }
			return false;
		case LO: // LO means we're fully logged on
			if( ChangeWorkingDirectory((&((const tstring)_T("/"))))!=FTP_OK )
				return false;
			if (TransferMode(CTransferMode::Stream())!=FTP_OK)
				return false;
			m_apCurrentTransfertMode=tmStream;

			if (!logonInfo.Feat())
				return true; 

			if (Feat() == FTP_OK)
				return true;
		}
	}
	return false;
}

/// Rename a file on the ftp server.
/// @param[in] strOldName Name of the file to rename.
/// @param[in] strNewName The new name for the file.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Rename(const tstring* strOldName, const tstring* strNewName)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(_T("RNFR ")+*strOldName, Reply) )
		return FTP_ERROR;

	if( Reply.Code().IsNegativeReply() )
		return FTP_NOTOK;
	else if( !Reply.Code().IsPositiveIntermediateReply() )
	{
		ASSERT( Reply.Code().IsPositiveCompletionReply() || Reply.Code().IsPositivePreliminaryReply() );
		return FTP_ERROR;
	}

	if( !SendCommand(_T("RNTO ")+*strNewName, Reply) )
		return FTP_ERROR;

	return SimpleErrorCheck(Reply);
}

/// feat 
int CFTPClient::Feat(void)
{

	if( !IsConnected() )
		return FTP_NOT_CONNECTED;


	CReply Reply;

	if( !SendCommand(_T("FEAT"))  )
		return FTP_ERROR;

	tstring strResponse("");

	if( !GetFeatMultipleResponseLine(strResponse) )
		return false;

	bool fRet = Reply.Set(&strResponse);

	m_notification.OnResponse(Reply);

	if (!fRet)
		return FTP_ERROR;

	if( Reply.Code().IsPositiveCompletionReply() )
		return FTP_OK;
	else if( Reply.Code().IsPositivePreliminaryReply() )
	{
		if( !GetResponse(Reply) || !Reply.Code().IsPositiveCompletionReply() )
			return FTP_ERROR;
	}
	else if( Reply.Code().IsNegativeReply() )
		return FTP_NOTOK;

	ASSERT( Reply.Code().IsPositiveIntermediateReply() );

	return FTP_ERROR;
}



/// Gets the directory listing of the ftp-server. Sends the LIST command to
/// the ftp-server.
/// @param[in] strPath Starting path for the list command.
/// @param[out] vFileList Returns a detailed list of the files and folders of the specified directory.
///                       vFileList contains CFTPFileStatus-Objects. These Objects provide a lot of
///                       information about the file/folder.
/// @param[in] fPasv see documentation of CFTPClient::Passive


int CFTPClient::List(const tstring* strPath, bool zip, bool fPasv)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	const CDatachannelCmd list = CDatachannelCmd::LIST() ;
	const CRepresentation ascii = CType::ASCII();

	if( !ExecuteDatachannelCommand(	(nsFTP::CDatachannelCmd *)&list,(tstring *)strPath,(nsFTP::CRepresentation *)&ascii,
	fPasv,false,zip,0,t_stream))
		return FTP_ERROR;

	return FTP_OK;
}

/// Gets the directory listing of the ftp-server. Sends the NLST command to
/// the ftp-server.
/// @param[in] strPath Starting path for the list command.
/// @param[out] vFileList Returns a simple list of the files and folders of the specified directory.
///                       vFileList contains CFTPFileStatus-Objects. Normally these Objects provide 
///                       a lot of information about the file/folder. But the NLST-command provide 
///                       only a simple list of the directory content (no specific information).
/// @param[in] fPasv see documentation of CFTPClient::Passive
int CFTPClient::NameList(const tstring* strPath, bool zip, bool fPasv, tstring &listOfFiles)
{
	//tstring listOfFiles;

	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	const CDatachannelCmd nlist = CDatachannelCmd::NLST() ;
	const CRepresentation ascii = CType::ASCII();

	if( !ExecuteDatachannelCommand((nsFTP::CDatachannelCmd *)&nlist,(tstring *)strPath,(nsFTP::CRepresentation *)&ascii,
	fPasv,false,zip,0,t_stream))
		return FTP_ERROR;

	m_coutputstream.GetLine(listOfFiles);

	return FTP_OK;
}

/// Gets a file from the ftp-server.
/// Uses C functions for file access (very fast).
/// @param[in] strRemoteFile Filename of the sourcefile on the ftp-server.
/// @param[in] strLocalFile Filename of the targetfile on the local computer.
/// @param[in] repType Representation Type (see documentation of CRepresentation)
/// @param[in] fPasv see documentation of CFTPClient::Passive
int CFTPClient::DownloadFile(const tstring* strRemoteFile, const tstring* strLocalFile, bool zip, bool overwrite,
							 bool fPasv) 
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	u64 lRemoteFileSize = 0;

	if (overwrite) {
		if( !cfile.Open(strLocalFile, _T("wb")) ){
			ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
			return FTP_ERROR;
		}
	}
	else {
		if( !cfile.Open(strLocalFile, m_fResumeIfPossible  ?_T("ab"):_T("wb")) ){
			ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
			return FTP_ERROR;
		}
		cfile.Seek(0, CFile::orEnd);

		FileSize(strRemoteFile, lRemoteFileSize);
	}


	m_notification.OnPreReceiveFile(strRemoteFile, strLocalFile, lRemoteFileSize);

	CRepresentation repType=CRepresentation(CType::Image());
	const CDatachannelCmd retr = CDatachannelCmd::RETR() ;

	const bool fRet=ExecuteDatachannelCommand(
	(nsFTP::CDatachannelCmd *)&retr,(tstring *)strRemoteFile,(nsFTP::CRepresentation *)&repType,
	fPasv,overwrite,zip, cfile.Tell(),	t_file);


	cfile.Close();
	m_notification.OnPostReceiveFile(strRemoteFile, strLocalFile, lRemoteFileSize);

	if (fRet) return FTP_OK;
	else return FTP_ERROR;
}

// Gets a file from the ftp-server.
/// Uses C functions for file access (very fast).
/// @param[in] strRemoteFile Filename of the sourcefile on the ftp-server.
/// @param[in] strLocalMemFile pointer of the targetfile on the local computer.
/// @param[in] strLocalMemFile length of the targetfile on the local computer.
/// @param[in] remoteFileLength length of the targetfile on the local computer.
/// @param[in] repType Representation Type (see documentation of CRepresentation)
/// @param[in] fPasv see documentation of CFTPClient::Passive
int CFTPClient::DownloadMemFile(const tstring* strRemoteFile, const void* strLocalMemFile, 
								u32 strLocalMemFileLen, u32 *remoteFileLength, bool zip,
							 bool fPasv) 
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	u64 lRemoteFileSize = 0;

	if( !cmemfile.Open(strLocalMemFile,strLocalMemFileLen) ){
		ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
		return FTP_ERROR;
	}

	const tstring  memfile ("memfile"); 
	m_notification.OnPreReceiveFile(strRemoteFile, &memfile, lRemoteFileSize);

	CRepresentation repType=CRepresentation(CType::Image());
	const CDatachannelCmd retr = CDatachannelCmd::RETR() ;

	const bool fRet=ExecuteDatachannelCommand((nsFTP::CDatachannelCmd *)&retr,
	(tstring *)strRemoteFile,(nsFTP::CRepresentation *)&repType,fPasv,	true,zip,cmemfile.Tell(),t_memfile);


	*remoteFileLength = cmemfile.Size();

	cmemfile.Close();
	m_notification.OnPostReceiveFile(strRemoteFile, &memfile, lRemoteFileSize);

	if (fRet) return FTP_OK;
	else return FTP_ERROR;
}

/// Puts a file on the ftp-server.
/// Uses C functions for file access (very fast).
/// @param[in] strLocalFile Filename of the the local sourcefile which to put on the ftp-server.
/// @param[in] strRemoteFile Filename of the targetfile on the ftp-server.
/// @param[in] fStoreUnique if true, the ftp command STOU is used for saving
///                         else the ftp command STOR is used.
/// @param[in] repType Representation Type (see documentation of CRepresentation)
/// @param[in] fPasv see documentation of CFTPClient::Passive
int CFTPClient::UploadFile(const tstring* strLocalFile, const tstring* strRemoteFile, bool zip, bool overwrite, 
						   bool fStoreUnique, 
						   bool fPasv) 
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	if( !cfile.Open(strLocalFile, _T("rb")) )
	{
		ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
		return FTP_ERROR;
	}

	u64 lRemoteFileSize = 0;
	u64 lLocalFileSize =0;
	bool fRet;

	CDatachannelCmd cmd(CDatachannelCmd::STOR());

	if (!overwrite) {
		if(m_fResumeIfPossible )
			FileSize(strRemoteFile, lRemoteFileSize);

		if(lRemoteFileSize > 0 )
			cmd = CDatachannelCmd::APPE();
		else if( fStoreUnique )
			cmd = CDatachannelCmd::STOU();

		cfile.Seek(0, CFile::orEnd);
		lLocalFileSize = cfile.Tell();
		cfile.Seek(lRemoteFileSize, CFile::orBegin);
	}
	else {
		cmd=CDatachannelCmd::STOR();
	}
	CRepresentation repType=CRepresentation(CType::Image());

    fRet=ExecuteDatachannelCommand(&cmd,strRemoteFile,&repType,fPasv,overwrite,zip,0,t_file);
	
	m_notification.OnPostSendFile(strLocalFile, strRemoteFile, lLocalFileSize);

	cfile.Close();

	if (fRet) return FTP_OK;
	else return FTP_ERROR;
}


/// Puts a file on the ftp-server.
/// Uses C functions for file access (very fast).
/// @param[in] strLocalFile Filename of the the local sourcefile which to put on the ftp-server.
/// @param[in] strRemoteFile Filename of the targetfile on the ftp-server.
/// @param[in] fStoreUnique if true, the ftp command STOU is used for saving
///                         else the ftp command STOR is used.
/// @param[in] repType Representation Type (see documentation of CRepresentation)
/// @param[in] fPasv see documentation of CFTPClient::Passive
int CFTPClient::UploadMemFile(const void * strLocalFileMem, const u32 strLocalFileLength, const tstring* strRemoteFile, bool zip, bool overwrite, 
						   bool fStoreUnique, 
						   bool fPasv) 
{
	tstring memfile ("memfile");

	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	if( !cmemfile.Open(&memfile, strLocalFileLength ))
	{
		ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
		return FTP_ERROR;
	}

	u64 lRemoteFileSize = 0;
	u64 lLocalFileSize =0;
	bool fRet;

	CDatachannelCmd cmd(CDatachannelCmd::STOR());

	if (!overwrite) {
		if(m_fResumeIfPossible )
			FileSize(strRemoteFile, lRemoteFileSize);

		if(lRemoteFileSize > 0 )
			cmd = CDatachannelCmd::APPE();
		else if( fStoreUnique )
			cmd = CDatachannelCmd::STOU();

		cmemfile.Seek(0, CMemFile::orEnd);
		lLocalFileSize = cfile.Tell();
		cmemfile.Seek(lRemoteFileSize, CMemFile::orBegin);
	}
	else {
		cmd=CDatachannelCmd::STOR();
	}
	CRepresentation repType=CRepresentation(CType::Image());

	fRet=ExecuteDatachannelCommand(	(nsFTP::CDatachannelCmd *)&cmd,(tstring *)strRemoteFile,
	(nsFTP::CRepresentation *)&repType,fPasv,overwrite,zip,0,t_memfile);

	m_notification.OnPostSendFile(&memfile, strRemoteFile, lLocalFileSize);

	cmemfile.Close();

	if (fRet) return FTP_OK;
	else return FTP_ERROR;
}

/// Executes a commando that result in a communication over the data port.
/// @param[in] crDatachannelCmd Command to be executeted.
/// @param[in] strPath Parameter for the command usually a path.
/// @param[in] representation see documentation of CFTPClient::CRepresentation
/// @param[in] fPasv see documentation of CFTPClient::Passive
//  @param[in] overwrite file 
/// @param[in] dwByteOffset Server marker at which file transfer is to be restarted.
/// @param[in] pObserver Object for observing the execution of the command.
int POPO=50;
bool CFTPClient::ExecuteDatachannelCommand(const CDatachannelCmd* crDatachannelCmd, const tstring* strPath ,
	const CRepresentation*representation, bool fPasv, bool overwrite, bool zip,	DWORD dwByteOffset,
	t_Obserser pObserver)
{

	if (!m_zip_supported_by_server && m_feat ) zip=false;

	if( m_fTransferInProgress ) 
		return false;

	if( !IsConnected() )
		return false;

	// check representation
	m_apCurrentRepresentation = CRepresentation(CType::ASCII());

	if( *representation!=m_apCurrentRepresentation )
	{
		// transmit representation to server
		if( RepresentationType(*representation)!=FTP_OK )
			return false;
		m_apCurrentRepresentation = *representation;
	}

	if (!zip){
		if (m_apCurrentTransfertMode == tmCompressed) {
			if (TransferMode(CTransferMode::Stream())!=FTP_OK)
				return false;
			m_apCurrentTransfertMode=tmStream;
		}
	}

	if( zip )
	{
		const CRepresentation targetRepresentation = CRepresentation(CType::Image());
		if (m_apCurrentRepresentation != targetRepresentation)
		{
			// transmit representation to server
			if( RepresentationType(targetRepresentation)!=FTP_OK )
				return false;
			m_apCurrentRepresentation = targetRepresentation;
		}
		if (m_apCurrentTransfertMode != tmCompressed) {
			if (TransferMode(CTransferMode::Compressed())!=FTP_OK)
				return false;
			m_apCurrentTransfertMode=tmCompressed;
		}
	}


	bool CompletionReply = false;
	if( fPasv )
	{
		{
			unsigned long   ulRemoteHostIP = 0;
			unsigned int  ushServerSock  = 0;

			// set passive mode
			// the ftp server opens a port and tell us the socket (ip address + port)
			// this socket is used for opening the data connection 
			if( Passive(ulRemoteHostIP, ushServerSock)!=FTP_OK )
				return false;

			// establish connection
			CSockAddr sockAddrTemp;

			if (m_sckDataConnection.Create(SOCK_STREAM)) {
				CSockAddr csaAddress(ulRemoteHostIP, ushServerSock); 
				if (!m_sckDataConnection.Connect(csaAddress)) {
					ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
				    m_sckDataConnection.Cleanup();
					return false;
				}

			}

			if (!overwrite)
			{
				// if resuming is activated then set offset
				if( m_fResumeIfPossible && 
					(*crDatachannelCmd==CDatachannelCmd::cmdSTOR || *crDatachannelCmd==CDatachannelCmd::cmdRETR || *crDatachannelCmd==CDatachannelCmd::cmdAPPE ) &&
					(dwByteOffset!=0 && Restart(dwByteOffset)!=FTP_OK) )
					return false;
			}

			// send FTP command RETR/STOR/NLST/LIST to the server
			CReply Reply;
			if( !SendCommand(GetCmdString(crDatachannelCmd, strPath), Reply) ||
				!Reply.Code().IsPositiveReply())
				return false;
sgIP_dbgprint("848") ;
			CompletionReply=Reply.Code().IsPositiveCompletionReply();
sgIP_dbgprint("850") ;

		}

	}
	else
	{

		{

			unsigned int ushLocalSock = 0;
			CompletionReply =false;

			// INADDR_ANY = ip address of localhost
			// second parameter "0" means that the WINSOCKAPI ask for a port
			if (m_sckDataConnection.Create(SOCK_STREAM)) // m_hSocket
			{
				CSockAddr csaAddressTemp((unsigned long int) (INADDR_ANY), POPO); // Set port to 100
				// POPO = POPO+10; 
				if( m_sckDataConnection.Bind(csaAddressTemp)) {
					//	  apSckServer->GetSockAddr(csaAddressTemp);
					ushLocalSock=csaAddressTemp.Port();
					if (m_sckDataConnection.Listen()){

						// get own ip address
						CSockAddr csaLocalAddress;
						m_sckDataConnection.GetSockAddr(csaLocalAddress);

						// transmit the socket (ip address + port) to the server
						// the ftp server establishes then the data connection
						if( DataPort(&csaLocalAddress.DottedDecimal(), ushLocalSock)!=FTP_OK )
							return false;

						if (!overwrite)
						{
							// if resuming is activated then set offset
							if( m_fResumeIfPossible && 
								(*crDatachannelCmd==CDatachannelCmd::cmdSTOR || *crDatachannelCmd==CDatachannelCmd::cmdRETR || *crDatachannelCmd==CDatachannelCmd::cmdAPPE ) &&
								(dwByteOffset!=0 && Restart(dwByteOffset)!=FTP_OK) )
								return false;
						}

						// send FTP command RETR/STOR/NLST/LIST to the server
						CReply Reply;
						if( !SendCommand(GetCmdString(crDatachannelCmd, strPath), Reply))
							return false;
						if (!Reply.Code().IsPositivePreliminaryReply() ) 
						{
							if (!Reply.Code().IsPositiveCompletionReply())
								return false;
							else 
								CompletionReply =true;
						}
						sgIP_dbgprint("Socket data accepted\n") ;

						if( !m_sckDataConnection.Accept() ) {
						sgIP_dbgprint("Socket not accepted\n") ;

							m_sckDataConnection.Cleanup();
							return false;
						}
						sgIP_dbgprint("Socket connect Ok\n") ;
						goto EverOk;
					}
				}
			}
			/*
			sock=socket(AF_INET,SOCK_DGRAM,0);
			sain.sin_family=AF_INET;
			sain.sin_port=htons(portnum);
			sain.sin_addr.s_addr=INADDR_ANY;
			bind(sock,(struct sockaddr *) &sain,sizeof(sain));
			rcvd=0;
			i=1;
			ioctl(sock,FIONBIO,&i);

			*/
			ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
			m_sckDataConnection.Cleanup();
			return false;

		}
	}

EverOk:
	const bool fTransferOK = TransferData(crDatachannelCmd, pObserver, zip);

	if (fTransferOK && (CDatachannelCmd::LIST() == *crDatachannelCmd)) {

	tstring strLine;
	CFTPListParse ftpListParser; // parse object 

	this->numberofFiles=0;
	m_coutputstream.SetStartPosition();

	while( m_coutputstream.GetNextLine(strLine) )
	{
		int currFile = this->numberofFiles;
		if (currFile == mCFTPFileStatusNbr) break;

		if( ftpListParser.Parse(DirList[currFile], strLine) )
		{
		   sgIP_dbgprint("add a new file") ;
		   if (this->numberofFiles < MaxNbFilesList()) {
			DirList[currFile].Path() = *strPath;
			this->numberofFiles++;
			}
		}
	}

	}

    // close the socket 
	m_sckDataConnection.Close();
 
	// get response from ftp server
	if ( !fTransferOK )
		return false;

	if (!CompletionReply) {
		CReply Reply;
		if( !GetResponse(Reply) || !Reply.Code().IsPositiveCompletionReply() )
			return false;
	}

	return true;
}

/// Executes a commando that result in a communication over the data port.
/// @param[in] crDatachannelCmd Command to be executeted.
/// @param[in] pObserver Object for observing the execution of the command.
/// @param[in] m_sckDataConnection Socket which is used for sending/receiving data.
bool CFTPClient::TransferData(const CDatachannelCmd* crDatachannelCmd, t_Obserser pObserver, 
							  bool zip)
{
	m_sckDataConnection.CurrentTransfer.iZlibLevel = this->iZlibLevel;

	switch( crDatachannelCmd->AsEnum() )
	{
	case CDatachannelCmd::cmdSTOR:
	case CDatachannelCmd::cmdSTOU:
	case CDatachannelCmd::cmdAPPE:
		{
			if (zip) {
				if( !SendDataZip(pObserver) )
					return false;
			} else {
				if( !SendData(pObserver) )
					return false;
			}
		}
		break;
	case CDatachannelCmd::cmdRETR:
	case CDatachannelCmd::cmdLIST:
	case CDatachannelCmd::cmdNLST:
		if (zip) {
			if( !ReceiveDataZip(pObserver) ){
				return false;
			}
		} else {
			if( !ReceiveData(pObserver) )
				return false;
		}
		break;
	default:
		ASSERT( false );
		return false;
	}
	return true;
}

/// Opens an active data connection.
/// @param[out] m_sckDataConnection
/// @param[in] crDatachannelCmd Command to be executeted.
/// @param[in] strPath Parameter for the command usually a path.
/// @param[in] dwByteOffset Server marker at which file transfer is to be restarted.



/// Opens a passive data connection.
/// @param[out] m_sckDataConnection
/// @param[in] crDatachannelCmd Command to be executeted.
/// @param[in] strPath Parameter for the command usually a path.
/// @param[in] dwByteOffset Server marker at which file transfer is to be restarted.

/// Sends data over a socket to the server.
/// @param[in] pObserver Object for observing the execution of the command.
/// @param[in] m_sckDataConnection Socket which is used for the send action.
bool CFTPClient::SendData(t_Obserser pObserver) 
{
	m_fTransferInProgress=true;

	int iNumWrite;
	size_t bytesRead;

	if (pObserver==t_file) cfile.OnPreBytesSend(m_vBuffer, m_vBufferSize, bytesRead);
	else if (pObserver==t_memfile) cmemfile.OnPreBytesSend(m_vBuffer, m_vBufferSize, bytesRead);
	else m_coutputstream.OnPreBytesSend(m_vBuffer, bytesRead);;

	while( !m_fAbortTransfer && bytesRead!=0 )
	{
		iNumWrite = m_sckDataConnection.Write(m_vBuffer, static_cast<int>(bytesRead), mc_uiTimeout);

		if(!bytesRead || !iNumWrite)
		{
			m_fTransferInProgress=false;
			ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
			m_sckDataConnection.Cleanup();
			return false;
		}


		ASSERT( iNumWrite == static_cast<int>(bytesRead) );

		m_notification.OnBytesSent(m_vBuffer, iNumWrite);

		if (pObserver==t_file) cfile.OnPreBytesSend(m_vBuffer, m_vBufferSize, bytesRead);
		else if (pObserver==t_memfile) cmemfile.OnPreBytesSend(m_vBuffer, m_vBufferSize, bytesRead);
		else m_coutputstream.OnPreBytesSend(m_vBuffer, bytesRead);
	}

	m_fTransferInProgress=false;
	if( m_fAbortTransfer )
	{
		Abort();
		return false;
	}
	return true;
}




bool CFTPClient::SendDataZip(t_Obserser pObserver) 
{
	int nFlush, nRet;
	char *pOutBuffer = NULL;
	char *pBuffer;
	unsigned int uiBufferSize = MAX_TransferBufferSize;
	struct CUnBlockingSocket::DataTransfer_t *pTransfer = &m_sckDataConnection.CurrentTransfer;

	pOutBuffer = TransferBufferZipSize;

	m_fTransferInProgress=true;
	int iNumWrite;
	size_t bytesRead;

	if( !m_sckDataConnection.InitZlib( pTransfer , CUnBlockingSocket::DOWNLOADING ) ){
		return false;
	}
	if (pObserver==t_file) cfile.OnPreBytesSend(m_vBuffer, m_vBufferSize, bytesRead);
	else if (pObserver==t_memfile) cmemfile.OnPreBytesSend(m_vBuffer, m_vBufferSize, bytesRead);
	else m_coutputstream.OnPreBytesSend(m_vBuffer, bytesRead);

	pBuffer = m_vBuffer; 
	pTransfer->zStream.next_in = (Bytef*) pBuffer;
	pTransfer->zStream.next_out = (Bytef*) pOutBuffer;
	pTransfer->zStream.avail_out = uiBufferSize;

	while( !m_fAbortTransfer && bytesRead!=0 )
	{
		if (pObserver==t_file) nFlush = cfile.Feof() ? Z_FINISH : Z_NO_FLUSH;
		else if (pObserver==t_memfile) nFlush = cmemfile.Feof() ? Z_FINISH : Z_NO_FLUSH;
		else nFlush = Z_FINISH ;


		nFlush = cfile.Feof() ? Z_FINISH : Z_NO_FLUSH;
		pTransfer->zStream.avail_in = bytesRead;
		pTransfer->zStream.next_in = (Bytef*) pBuffer;
		do
		{
			nRet = deflate( &pTransfer->zStream, nFlush );
			if( nRet == Z_STREAM_ERROR ) {
				break;
			}
			if( pTransfer->zStream.avail_out == 0 || nRet == Z_STREAM_END ) {

				iNumWrite = m_sckDataConnection.Write(pOutBuffer, uiBufferSize - pTransfer->zStream.avail_out, mc_uiTimeout);
				if(!iNumWrite)
				{
					m_fTransferInProgress=false;
					ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
					m_sckDataConnection.Cleanup();
					return false;
				}

				m_notification.OnBytesSent(m_vBuffer, iNumWrite);

				pTransfer->zStream.next_out = (Bytef*) pOutBuffer;
				pTransfer->zStream.avail_out = uiBufferSize;
			}
		} while( pTransfer->zStream.avail_in != 0 || ( nFlush == Z_FINISH && nRet == Z_OK ) );

		if( nRet == Z_STREAM_ERROR || nFlush == Z_FINISH ){
			break; 
		}

		if (pObserver==t_file) cfile.OnPreBytesSend(m_vBuffer, m_vBufferSize, bytesRead);
		else if (pObserver==t_memfile) cmemfile.OnPreBytesSend(m_vBuffer, m_vBufferSize, bytesRead);
		else m_coutputstream.OnPreBytesSend(m_vBuffer, bytesRead);
		pTransfer->zStream.next_in = (Bytef*) pBuffer;

	}
	deflateEnd( &pTransfer->zStream );
	m_fTransferInProgress=false;
	if( m_fAbortTransfer )
	{
		Abort();
		return false;
	}

	return true;
}



/// Receives data over a socket from the server.
/// @param[in] pObserver Object for observing the execution of the command.
/// @param[in] m_sckDataConnection Socket which is used for receiving the data.
bool CFTPClient::ReceiveData(t_Obserser pObserver) 
{
	m_fTransferInProgress = true;

	m_notification.OnBeginReceivingData();
	u32 iNumRead=m_sckDataConnection.Receive(m_vBuffer, m_vBufferSize, mc_uiTimeout, true);
	u32 lTotalBytes = iNumRead;
	while( !m_fAbortTransfer && iNumRead!=0 )
	{
		m_notification.OnBytesReceived(m_vBuffer, iNumRead);

		if (pObserver==t_file) {
			cfile.OnBytesReceived(m_vBuffer, iNumRead); // write the result
			iNumRead=m_sckDataConnection.Receive(m_vBuffer, m_vBufferSize, mc_uiTimeout, true);
		}
		else if (pObserver==t_memfile) {
			cmemfile.OnBytesReceived(m_vBuffer, iNumRead); // write the result
			iNumRead=m_sckDataConnection.Receive(m_vBuffer, m_vBufferSize, mc_uiTimeout, true);
		}
		else {
			if (m_vBufferSize-lTotalBytes > 10) { 
			iNumRead=m_sckDataConnection.Receive(m_vBuffer+lTotalBytes, m_vBufferSize-lTotalBytes, mc_uiTimeout, true);
			}
			else iNumRead=0;
		}
		if(!iNumRead && !lTotalBytes)
		{
			m_fTransferInProgress=false;
			ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
			m_sckDataConnection.Cleanup();
			return false;
		}
		lTotalBytes += iNumRead;
	}
	if (pObserver==t_stream) m_coutputstream.OnBytesReceived(m_vBuffer, lTotalBytes);         // write

	m_notification.OnEndReceivingData(lTotalBytes);

	m_fTransferInProgress=false;
	if( m_fAbortTransfer )
	{
		Abort();
		return false;
	}
	return true;
}




extern char TransferBufferZipSize[];


/// Receives data over a socket from the server.
/// @param[in] pObserver Object for observing the execution of the command.
/// @param[in] m_sckDataConnection Socket which is used for receiving the data.
bool CFTPClient::ReceiveDataZip(t_Obserser pObserver) 
{
	int nFlush, nRet;
	char *pOutBuffer = NULL;
	char *pBuffer = TransferBufferZipSize;
	const unsigned int uiBufferSize = m_vBufferSize;
	struct CUnBlockingSocket::DataTransfer_t *pTransfer = &m_sckDataConnection.CurrentTransfer;
	pOutBuffer = m_vBuffer;

	sgIP_dbgprint("ReceiveDataZip");

	if( !pOutBuffer || !pBuffer ) {
		return false;
	}
	if( !m_sckDataConnection.InitZlib( pTransfer , CUnBlockingSocket::UPLOADING ) ){
		return false;
	}
	pTransfer->zStream.next_out = (Bytef*) pOutBuffer;
	pTransfer->zStream.avail_out = uiBufferSize;


	m_fTransferInProgress = true;

	m_notification.OnBeginReceivingData();

	int iNumRead=m_sckDataConnection.Receive(pBuffer, uiBufferSize, mc_uiTimeout, true);
	long lTotalBytes = iNumRead;
	long lTotalStored = 0;

	if(!iNumRead)
	{
		m_fTransferInProgress=false;
		ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
		m_sckDataConnection.Cleanup();
		return false;
	}

	while( !m_fAbortTransfer && iNumRead!=0 )
	{
		pTransfer->zStream.avail_in = iNumRead;
		pTransfer->zStream.next_in = (Bytef*) pBuffer;
		nFlush = !iNumRead ? Z_NO_FLUSH : Z_FINISH;
		do
		{
			nRet = inflate( &pTransfer->zStream, nFlush );
			if( nRet != Z_OK && nRet != Z_STREAM_END && nRet != Z_BUF_ERROR ){
				break; // Zlib error
			}
			if( iNumRead == 0 && nRet == Z_BUF_ERROR ) {
				break; // transfer has been interrupt by the client.
			}
			if( pTransfer->zStream.avail_out == 0 || nRet == Z_STREAM_END ) {
				lTotalStored += uiBufferSize - pTransfer->zStream.avail_out;
				if (pObserver==t_file) cfile.OnBytesReceived(m_vBuffer, uiBufferSize - pTransfer->zStream.avail_out); // write the result
				else if (pObserver==t_memfile) cmemfile.OnBytesReceived(m_vBuffer, uiBufferSize - pTransfer->zStream.avail_out);
				m_notification.OnBytesReceived(m_vBuffer, uiBufferSize - pTransfer->zStream.avail_out);

				pTransfer->zStream.next_out = (Bytef*) pOutBuffer;
				pTransfer->zStream.avail_out = uiBufferSize;
			}
		} while( pTransfer->zStream.avail_in != 0 );

		if( nRet == Z_STREAM_END ){
			break;
		}

		iNumRead=m_sckDataConnection.Receive(pBuffer, uiBufferSize, mc_uiTimeout, true);
		if(!iNumRead)
		{
			m_fTransferInProgress=false;
			ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
			m_sckDataConnection.Cleanup();
			return false;
		}

		lTotalBytes += iNumRead;
	}
	m_notification.OnEndReceivingData(lTotalBytes);

	if (pObserver==t_stream) m_coutputstream.OnBytesReceived(m_vBuffer, lTotalBytes);         // write
	m_fTransferInProgress=false;
	if( m_fAbortTransfer )
	{
		Abort();
		return false;
	}
	deflateEnd( &pTransfer->zStream );

	//endofstore:

	sgIP_dbgprint("ReceiveData %d\n",__LINE__);
	return true;
}




/// Returns the command string for a specific command.
/// @param[in] crDatachannelCmd Command for which the string should be returned.
/// @param[in] strPath Parameter which have to be added to the command.
tstring CFTPClient::GetCmdString(const CDatachannelCmd* crDatachannelCmd, const tstring* strPath)
{
	switch( crDatachannelCmd->AsEnum() )
	{
	case CDatachannelCmd::cmdLIST: return strPath->empty()?_T("LIST"):_T("LIST ") + *strPath;
	case CDatachannelCmd::cmdNLST: return strPath->empty()?_T("NLST"):_T("NLST ") + *strPath;
	case CDatachannelCmd::cmdSTOR: return _T("STOR ") + *strPath;
	case CDatachannelCmd::cmdSTOU: return _T("STOU ") + *strPath;
	case CDatachannelCmd::cmdRETR: return _T("RETR ") + *strPath;
	case CDatachannelCmd::cmdAPPE: return _T("APPE ") + *strPath;
	default:
		ASSERT( false );
	}
	return _T("");
}

/// Sends a command to the server.
/// @param[in] strCommand Command to send.
bool CFTPClient::SendCommand(const tstring& strCommand) 
{
	tstring strCommandFull = strCommand + "\r\n";
	if( !IsConnected() )
		return false;

	m_notification.OnSendCommand(&strCommand);

	if (!m_apSckControlConnection.Write(strCommandFull.c_str(), strCommandFull.length(), mc_uiTimeout))
	{
		ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
		const_cast<CFTPClient*>(this)->m_apSckControlConnection.Cleanup();
		return false;
	}
	return true;
}

/// Sends a command to the server.
/// @param[in]  strCommand Command to send.
/// @param[out] Reply The Reply of the server to the sent command.
bool CFTPClient::SendCommand(const tstring& strCommand, CReply& Reply) 
{
	if( !SendCommand(strCommand) || !GetResponse(Reply) )
		return false;
	return true;
}

/// This function gets the server response.
/// A server response can exists of more than one line.
/// @param[out] Reply Reply of the server to a command.
bool CFTPClient::GetResponse(CReply& Reply) 
{
	tstring strResponse("");

	if( !GetSingleResponseLine(strResponse) )
		return false;

	bool fRet = Reply.Set(&strResponse);

	m_notification.OnResponse(Reply);

	return fRet;
}

/// Reads a multiple response line from the server control channel.
/// @param[out] strResponse Response of the server as string.
bool CFTPClient::GetFeatMultipleResponseLine(tstring& strResponse) 
{
	if( !IsConnected() )
		return false;

	int iNum=0;
	
	m_vBuffer[0] = 0; 
	
	// get firstline
	iNum=m_apSckControlConnection.ReceiveLine(m_vBuffer, m_vBufferSize-1, mc_uiTimeout, true);
	if (!iNum)
	{
		ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
		(this)->m_apSckControlConnection.Cleanup();
		return false;
	}

	strResponse = tstring(m_vBuffer);
	
	// remove CrLf if exists
	if( strResponse.length()> 1 && strResponse.substr(strResponse.length()-2)==_T("\r\n") )
		strResponse.erase(strResponse.length()-2, 2);

	m_zip_supported_by_server=false;

	do
	{
		iNum=m_apSckControlConnection.ReceiveLine(m_vBuffer, m_vBufferSize-1, mc_uiTimeout, true);
		if (!iNum)
		{
			ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
			(this)->m_apSckControlConnection.Cleanup();
			return false;
		}
		tstring strTemp(m_vBuffer);
	
		if (strTemp.find("MODE Z")!=-1) { //Mode Z allowed
			m_zip_supported_by_server=true; 
		}
		if (strTemp.find("SIZE")!=-1) { //Mode Z allowed
			m_size_supported_by_server=true; 
		}		
		if (strTemp.find(strResponse.substr(0,3))!=-1)  // last message 
			return true;
	} while(iNum);

	return false;
}

/// Reads a single response line from the server control channel.
/// @param[out] strResponse Response of the server as string.
bool CFTPClient::GetSingleResponseLine(tstring& strResponse) 
{
	if( !IsConnected() )
		return false;

	int iNum=0;
	
	m_vBuffer[0] = 0;
	
	// get firstline
	iNum=m_apSckControlConnection.ReceiveLine(m_vBuffer, m_vBufferSize-1, mc_uiTimeout, true);

	if (!iNum)
	{
		ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
		(this)->m_apSckControlConnection.Cleanup();
		return false;
	}

	strResponse = tstring(m_vBuffer);
	
	// remove CrLf if exists
	if( strResponse.length()> 1 && strResponse.substr(strResponse.length()-2)==_T("\r\n") )
		strResponse.erase(strResponse.length()-2, 2);


	if( !( strResponse.length()>3 && strResponse[3] == '-')) {	// it's not a multi-lineS response, wait for end response
			return true;
	}

restartSingleResponseLine:

	// internal buffer is empty ==> get response from ftp-server
	iNum=0;

	m_vBuffer[0] = 0;

	do
	{
		iNum=m_apSckControlConnection.ReceiveLine(m_vBuffer, m_vBufferSize-1, mc_uiTimeout, true);

	} while(iNum !=0);

	if (!iNum)
	{
		ReportError( CCnv::ConvertToTString(__FILE__), __LINE__);
		(this)->m_apSckControlConnection.Cleanup();
		return false;
	}


	if (iNum)
	{
	tstring strnewResponse(m_vBuffer);
	
	// remove CrLf if exists
	if( strnewResponse.length()> 1 && strnewResponse.substr(strnewResponse.length()-2)==_T("\r\n") )
		strnewResponse.erase(strnewResponse.length()-2, 2);

	if(  strnewResponse.length()>3 && strnewResponse[3] == '-') {	// it's a multi-lineS response, wait for end response
			goto restartSingleResponseLine;
	}
	}
		

	return true;
}

/// Executes the ftp command CDUP (change to parent directory).
/// This command is a special case of CFTPClient::ChangeWorkingDirectory 
/// (CWD), and is  included to simplify the implementation of programs for 
/// transferring directory trees between operating systems having different 
/// syntaxes for naming the parent directory.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::ChangeToParentDirectory()
{
	CReply Reply;
	if( !SendCommand(_T("CDUP"), Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command QUIT.
/// This command terminates a USER and if file transfer is not in progress, 
/// the server closes the control connection. If  file transfer is in progress, 
/// the connection will remain open for result response and the server will 
/// then close it.
/// If the user-process is transferring files for several USERs but does not 
/// wish to close and then reopen connections for each, then the REIN command 
/// should be used instead of QUIT.
/// An unexpected close on the control connection will cause the server to take 
/// the effective action of an abort (ABOR) and a logout.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Logout()
{
	CReply Reply;
	if( !SendCommand(_T("QUIT"), Reply) )
		return FTP_ERROR;

	CloseControlChannel();

	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command PASV. Set the passive mode.
/// This command requests the server-DTP (data transfer process) on a data to 
/// "listen"  port (which is not its default data port) and to wait for a 
/// connection rather than initiate one upon receipt of a transfer command. 
/// The response to this command includes the host and port address this 
/// server is listening on.
/// @param[out] ulIpAddress IP address the server is listening on.
/// @param[out] ushPort Port the server is listening on.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Passive(unsigned long& ulIpAddress, unsigned int& ushPort)
{
	CReply Reply;
	if( !SendCommand(_T("PASV"), Reply) )
		return FTP_ERROR;

	if( Reply.Code().IsPositiveCompletionReply() )
	{
		if( !GetIpAddressFromResponse(&Reply.Value(), ulIpAddress, ushPort) )
			return FTP_ERROR;
	}

	return SimpleErrorCheck(Reply);
}

/// Parses a response string and extracts the ip address and port information.
/// @param[in]  strResponse The response string of a ftp server which holds 
///                         the ip address and port information.
/// @param[out] ulIpAddress Buffer for the ip address.
/// @param[out] ushPort     Buffer for the port information.
/// @retval true  Everything went ok.
/// @retval false An error occurred (invalid format).
bool CFTPClient::GetIpAddressFromResponse(const tstring* strResponse, unsigned long& ulIpAddress, unsigned int& ushPort)
{
	// parsing of ip-address and port implemented with a finite state machine
	// ...(192,168,1,1,3,44)...
	enum T_enState { state0, state1, state2, state3, state4 } enState = state0;

	tstring strIpAddress, strPort;
	unsigned int ushTempPort = 0;
	unsigned long  ulTempIpAddress = 0;
	int iCommaCnt = 4;
	for( tstring::const_iterator it=strResponse->begin(); it!=strResponse->end(); ++it )
	{
		switch( enState )
		{
		case state0:
			if( *it == _T('(') )
				enState = state1;
			break;
		case state1:
			if( *it == _T(',') )
		 {
			 if( --iCommaCnt == 0 )
			 {
				 enState = state2;
				 ulTempIpAddress += CCnv::TStringToLong(strIpAddress.c_str());
			 }
			 else
			 {
				 ulTempIpAddress += CCnv::TStringToLong(strIpAddress.c_str())<<8*iCommaCnt;
				 strIpAddress="";
			 }
		 }
			else
		 {
			 if( !tisdigit(*it) )
				 return false;
			 strIpAddress += *it;
		 }
			break;
		case state2:
			if( *it == _T(',') )
		 {
			 ushTempPort = static_cast<unsigned int>(CCnv::TStringToLong(strPort.c_str())<<8);
			 strPort="";
			 enState = state3;
		 }
			else
		 {
			 if( !tisdigit(*it) )
				 return false;
			 strPort += *it;
		 }
			break;
		case state3:
			if( *it == _T(')') )
		 {
			 // compiler warning if using +=operator
			 ushTempPort = ushTempPort + static_cast<unsigned int>(CCnv::TStringToLong(strPort.c_str()));
			 enState = state4;
		 }
			else
		 {
			 if( !tisdigit(*it) )
				 return false;
			 strPort += *it;
		 }
		}
	}

	if( enState==state4 )
	{
		ulIpAddress = ulTempIpAddress;
		ushPort = ushTempPort;
	}

	return enState==state4;
}

/// Executes the ftp command ABOR.
/// This command tells the server to abort the previous FTP service command 
/// and any associated transfer of data.  The abort command may require 
/// "special action", as discussed in the Section on FTP Commands, to force 
/// recognition by the server.  No action is to be taken if the previous 
/// command has been completed (including data transfer).  The control
/// connection is not to be closed by the server, but the data connection 
/// must be closed.
/// There are two cases for the server upon receipt of this command:<BR>
/// (1) the FTP service command was already completed, or <BR>
/// (2) the FTP service command is still in progress.<BR>
/// In the first case, the server closes the data connection (if it is open)
/// and responds with a 226 reply, indicating that the abort command was 
/// successfully processed.
/// In the second case, the server aborts the FTP service in progress and 
/// closes the data connection, returning a 426 reply to indicate that the 
/// service request terminated abnormally.  The server then sends a 226 reply,
/// indicating that the abort command was successfully processed.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Abort()
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	if( m_fTransferInProgress )
	{
		m_fAbortTransfer = true;
		return FTP_OK;
	}

	m_fAbortTransfer = false;
	CReply Reply;
	if( !SendCommand(_T("ABOR"), Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}
/// Executes the ftp command PWD (PRINT WORKING DIRECTORY)
/// This command causes the name of the current working directory 
/// to be returned in the reply.
int CFTPClient::PrintWorkingDirectory()
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(_T("PWD"), Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command SYST (SYSTEM)
/// This command is used to find out the type of operating system at the server.
/// The reply shall have as its first word one of the system names listed in the 
/// current version of the Assigned Numbers document [Reynolds, Joyce, and 
/// Jon Postel, "Assigned Numbers", RFC 943, ISI, April 1985.].
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::System()
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(_T("SYST"), Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command NOOP
/// This command does not affect any parameters or previously entered commands. 
/// It specifies no action other than that the server send an FTP_OK reply.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Noop()
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(_T("NOOP"), Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command PORT (DATA PORT)
/// The argument is a HOST-PORT specification for the data port to be used in data 
/// connection. There are defaults for both the user and server data ports, and 
/// under normal circumstances this command and its reply are not needed.  If
/// this command is used, the argument is the concatenation of a 32-bit internet 
/// host address and a 16-bit TCP port address.
/// @param[in] strHostIP IP-address like xxx.xxx.xxx.xxx
/// @param[in] uiPort 16-bit TCP port address.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::DataPort(const tstring* strHostIP, unsigned int ushPort)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	tstring strPortCmd;
	// convert the port number to 2 bytes + add to the local IP
	strPortCmd = CMakeString() << _T("PORT ") << *strHostIP << _T(",") << (ntohs(ushPort>>8)) << _T(",") << ntohs(ushPort&0xFF);

	ReplaceStr(strPortCmd, _T("."), _T(","));

	CReply Reply;
	if( !SendCommand(strPortCmd, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command TYPE (REPRESENTATION TYPE)
/// see Documentation of nsFTP::CRepresentation
/// @param[in] representation see Documentation of nsFTP::CRepresentation
/// @param[in] iSize Indicates Bytesize for type LocalByte.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::RepresentationType(const CRepresentation& representation, DWORD dwSize)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	tstring strCmd;
	switch( representation.Type().AsEnum() )
	{
	case CType::tyASCII:     strCmd = _T("TYPE A");                             break;
	case CType::tyEBCDIC:    strCmd = _T("TYPE E");                             break;
	case CType::tyImage:     strCmd = _T("TYPE I");                             break;
	case CType::tyLocalByte: strCmd = CMakeString() << _T("TYPE L ") << dwSize; break;
	default:
		ASSERT( false );
		return FTP_ERROR;
	}

	if( representation.Type()==CType::tyASCII || 
		representation.Type()==CType::tyEBCDIC )
	{
		switch( representation.Format().AsEnum() )
		{
		case CTypeFormat::tfNonPrint:        strCmd += _T(" N"); break;
		case CTypeFormat::tfTelnetFormat:    strCmd += _T(" T"); break;
		case CTypeFormat::tfCarriageControl: strCmd += _T(" C"); break;
		default:
			ASSERT( false );
		}
	}

	CReply Reply;
	if( !SendCommand(strCmd, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command CWD (CHANGE WORKING DIRECTORY)
/// This command allows the user to work with a different directory or dataset 
/// for file storage or retrieval without altering his login or accounting 
/// information. Transfer parameters are similarly unchanged.
/// @param[in] strDirectory Pathname specifying a directory or other system 
///                         dependent file group designator.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::ChangeWorkingDirectory(const tstring* strDirectory)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	ASSERT( !strDirectory->empty() );
	CReply Reply;
	if( !SendCommand(_T("CWD ")+*strDirectory, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command MKD (MAKE DIRECTORY)
/// This command causes the directory specified in the pathname to be created 
/// as a directory (if the pathname is absolute) or as a subdirectory of the 
/// current working directory (if the pathname is relative).
/// @pararm[in] strDirectory Pathname specifying a directory to be created.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::MakeDirectory(const tstring* strDirectory)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	ASSERT( !strDirectory->empty() );
	CReply Reply;
	if( !SendCommand(_T("MKD ")+*strDirectory, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command SITE (SITE PARAMETERS)
/// This command is used by the server to provide services specific to his 
/// system that are essential to file transfer but not sufficiently universal 
/// to be included as commands in the protocol.  The nature of these services 
/// and the specification of their syntax can be stated in a reply to the HELP 
/// SITE command.
/// @param[in] strCmd Command to be executed.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::SiteParameters(const tstring* strCmd)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	ASSERT( !strCmd->empty() );
	CReply Reply;
	if( !SendCommand(_T("SITE ")+*strCmd, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command HELP
/// This command shall cause the server to send helpful information regarding 
/// its implementation status over the control connection to the user.
/// The command may take an argument (e.g., any command name) and return more 
/// specific information as a response.  The reply is type 211 or 214.
/// It is suggested that HELP be allowed before entering a USER command. The 
/// server may use this reply to specify site-dependent parameters, e.g., in 
/// response to HELP SITE.
/// @param[in] strTopic Topic of the requested help.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Help(const tstring* strTopic)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(strTopic->empty()?_T("HELP"):_T("HELP ")+*strTopic, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command DELE (DELETE)
/// This command causes the file specified in the pathname to be deleted at the 
/// server site.  If an extra level of protection is desired (such as the query, 
/// "Do you really wish to delete?"), it should be provided by the user-FTP process.
/// @param[in] strFile Pathname of the file to delete.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Delete(const tstring* strFile)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	ASSERT( !strFile->empty() );
	CReply Reply;
	if( !SendCommand(_T("DELE ")+*strFile, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command RMD (REMOVE DIRECTORY)
/// This command causes the directory specified in the pathname to be removed 
/// as a directory (if the pathname is absolute) or as a subdirectory of the 
/// current working directory (if the pathname is relative).
/// @param[in] strDirectory Pathname of the directory to delete.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::RemoveDirectory(const tstring* strDirectory)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	ASSERT( !strDirectory->empty() );
	CReply Reply;
	if( !SendCommand(_T("RMD ")+*strDirectory, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command STRU (FILE STRUCTURE)
/// see documentation of nsFTP::CStructure
/// The default structure is File.
/// @param[in] crStructure see Documentation of nsFTP::CStructure
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::FileStructure(const CStructure& crStructure)
{
	if( !IsConnected() )
		return false;

	tstring strStructure;
	switch( crStructure.AsEnum() )
	{
	case CStructure::scFile:   strStructure=_T("F"); break;
	case CStructure::scRecord: strStructure=_T("R"); break;
	case CStructure::scPage:   strStructure=_T("P"); break;
	default:
		ASSERT( false );
		return FTP_ERROR;
	}

	CReply Reply;
	if( !SendCommand(_T("STRU ")+strStructure, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command MODE (TRANSFER MODE)
/// see documentation of nsFTP::CTransferMode
/// The default transfer mode is Stream.
/// @param[in] crTransferMode see Documentation of nsFTP::CTransferMode
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::TransferMode(const CTransferMode& crTransferMode)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	tstring strMode;
	switch( crTransferMode.AsEnum() )
	{
	case CTransferMode::tmStream:      strMode=_T("S"); break;
	case CTransferMode::tmBlock:       strMode=_T("B"); break;
	case CTransferMode::tmCompressed:  strMode=_T("Z"); break;
	default:
		ASSERT( false );
		return FTP_ERROR;
	}

	CReply Reply;
	if( !SendCommand(_T("MODE ")+strMode, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}


/// Executes the ftp command MODE (TRANSFER MODE)
/// see documentation of nsFTP::OptionsModeZip
/// @param[in] crTransferMode see Documentation of nsFTP::CTransferMode
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::OptionsModeZip( const unsigned int Level )
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	tstring strLevel;

	switch (Level)
	{
	case 1 : strLevel=_T("1"); break;
	case 2 : strLevel=_T("2"); break;
	case 3 : strLevel=_T("3"); break;
	case 4 : strLevel=_T("4"); break;
	case 5 : strLevel=_T("5"); break;
	case 6 : strLevel=_T("6"); break;
	case 7 : strLevel=_T("7"); break;
	case 8 : strLevel=_T("8"); break;
	case 9 : strLevel=_T("9"); break;
	}

	CReply Reply;
	if( !SendCommand(_T("OPTS MODE Z LEVEL ")+strLevel, Reply) ) 
		return FTP_ERROR;
	int ret = SimpleErrorCheck(Reply);

	if (ret == FTP_OK)
		this->iZlibLevel = Level;

	return ret;
}


/// Executes the ftp command STAT (STATUS)
/// This command shall cause a status response to be sent over the control 
/// connection in the form of a reply. The command may be sent during a file 
/// transfer (along with the Telnet IP and Synch signals--see the Section on 
/// FTP Commands) in which case the server will respond with the status of the
/// operation in progress, or it may be sent between file transfers. In the 
/// latter case, the command may have an argument field. 
/// @param[in] strPath If the argument is a pathname, the command is analogous 
///                    to the "list" command except that data shall be transferred 
///                    over the control connection. If a partial pathname is 
///                    given, the server may respond with a list of file names or 
///                    attributes associated with that specification. If no argument 
///                    is given, the server should return general status information 
///                    about the server FTP process. This should include current 
///                    values of all transfer parameters and the status of connections.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Status(const tstring* strPath)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(strPath->empty()?_T("STAT"):_T("STAT ")+*strPath, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command ALLO (ALLOCATE)
/// This command may be required by some servers to reserve sufficient storage 
/// to accommodate the new file to be transferred.
/// @param[in] iReserveBytes The argument shall be a decimal integer representing 
///                          the number of bytes (using the logical byte size) of 
///                          storage to be reserved for the file. For files sent 
///                          with record or page structure a maximum record or page
///                          size (in logical bytes) might also be necessary; this 
///                          is indicated by a decimal integer in a second argument 
///                          field of the command.
/// @pararm[in] piMaxPageOrRecordSize This second argument is optional. This command 
///                          shall be followed by a STORe or APPEnd command.
///                          The ALLO command should be treated as a NOOP (no operation)
///                          by those servers which do not require that the maximum 
///                          size of the file be declared beforehand, and those servers 
///                          interested in only the maximum record or page size should 
///                          accept a dummy value in the first argument and ignore it.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Allocate(int iReserveBytes, const int* piMaxPageOrRecordSize/*=NULL*/)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	tstring strCmd;
	if( piMaxPageOrRecordSize==NULL )
		strCmd = CMakeString() << _T("ALLO ") << iReserveBytes;
	else
		strCmd = CMakeString() << _T("ALLO ") << iReserveBytes << _T(" R ") << *piMaxPageOrRecordSize;

	CReply Reply;
	if( !SendCommand(strCmd, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command SMNT ()
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::StructureMount(const tstring* strPath)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(_T("SMNT ")+*strPath, Reply) )
		return FTP_ERROR;
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command (STRUCTURE MOUNT)
/// This command allows the user to mount a different file system data structure 
/// without altering his login or accounting information. Transfer parameters 
/// are similarly unchanged.  The argument is a pathname specifying a directory 
/// or other system dependent file group designator.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Reinitialize()
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(_T("REIN"), Reply) )
		return FTP_ERROR;

	if( Reply.Code().IsPositiveCompletionReply() )
		return FTP_OK;
	else if( Reply.Code().IsPositivePreliminaryReply() )
	{
		if( !GetResponse(Reply) || !Reply.Code().IsPositiveCompletionReply() )
			return FTP_ERROR;
	}
	else if( Reply.Code().IsNegativeReply() )
		return FTP_NOTOK;

	ASSERT( Reply.Code().IsPositiveIntermediateReply() );
	return FTP_ERROR;
}

/// Executes the ftp command REST (RESTART)
/// This command does not cause file transfer but skips over the file to the 
/// specified data checkpoint. This command shall be immediately followed
/// by the appropriate FTP service command which shall cause file transfer 
/// to resume.
/// @param[in] dwPosition Represents the server marker at which file transfer 
///                       is to be restarted.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::Restart(DWORD dwPosition)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	tstring strCmd;
	strCmd = CMakeString() << _T("REST ") << dwPosition;

	CReply Reply;
	if( !SendCommand(strCmd, Reply) )
		return FTP_ERROR;

	if( Reply.Code().IsPositiveIntermediateReply() )
		return FTP_OK;
	else if( Reply.Code().IsNegativeReply() )
		return FTP_NOTOK;

	ASSERT( Reply.Code().IsPositiveReply() );

	return FTP_ERROR;
}

/// Executes the ftp command SIZE
/// Return size of file.
/// SIZE is not specified in RFC 959.
/// @param[in] Pathname of a file.
/// @param[out] Size of the file specified in pathname.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::FileSize(const tstring* strPath, u64& lSize)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	CReply Reply;
	if( !SendCommand(_T("SIZE ")+*strPath, Reply) )
		return FTP_ERROR;
	lSize = CCnv::TStringToLong(Reply.Value().substr(4).c_str());
	return SimpleErrorCheck(Reply);
}

/// Executes the ftp command MDTM
/// Show last modification time of file.
/// MDTM is not specified in RFC 959.
/// @param[in] strPath Pathname of a file.
/// @param[out] strModificationTime Modification time of the file specified in pathname.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::FileModificationTime(const tstring* strPath, tstring& strModificationTime)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	strModificationTime.erase();

	CReply Reply;
	if( !SendCommand(_T("MDTM ")+*strPath, Reply) )
		return FTP_ERROR;

	if( Reply.Value().length()>=18 )
	{
		tstring strTemp(Reply.Value().substr(4));
		size_t iPos=strTemp.find(_T('.'));
		if( iPos>-1 )
			strTemp = strTemp.substr(0, iPos);
		if( strTemp.length()==14 )
			strModificationTime=strTemp;
	}

	if( strModificationTime.empty() )
		return FTP_ERROR;

	return SimpleErrorCheck(Reply);
}

/// Show last modification time of file.
/// @param[in] strPath Pathname of a file.
/// @param[out] tmModificationTime Modification time of the file specified in pathname.
/// @return see return values of CFTPClient::SimpleErrorCheck
int CFTPClient::FileModificationTime(const tstring* strPath, tm& tmModificationTime)
{
	if( !IsConnected() )
		return FTP_NOT_CONNECTED;

	tstring strTemp;
	const int iRet = FileModificationTime(strPath, strTemp);

	memset(&tmModificationTime, 0, sizeof(tmModificationTime));
	if( iRet==FTP_OK )
	{
		tmModificationTime.tm_year = CCnv::TStringToLong(strTemp.substr(0, 4).c_str());
		tmModificationTime.tm_mon  = CCnv::TStringToLong(strTemp.substr(4, 2).c_str());
		tmModificationTime.tm_mday = CCnv::TStringToLong(strTemp.substr(6, 2).c_str());
		tmModificationTime.tm_hour = CCnv::TStringToLong(strTemp.substr(8, 2).c_str());
		tmModificationTime.tm_min  = CCnv::TStringToLong(strTemp.substr(10, 2).c_str());
		tmModificationTime.tm_sec  = CCnv::TStringToLong(strTemp.substr(12).c_str());
	}
	return iRet;
}

/// Notifies all observers that an error occurred.
/// @param[in] strErrorMsg Error message which is reported to all observers.
/// @param[in] Name of the sourcecode file where the error occurred.
/// @param[in] Line number in th sourcecode file where the error occurred.
void CFTPClient::ReportError(const tstring& strFile, DWORD dwLineNr) 
{

	m_notification.OnInternalError(strFile, dwLineNr);
	sleep(3);

}


CFTPClient::~CFTPClient()
{

}

