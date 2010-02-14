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

// other includes


#include "FTPClientconfig.h"
#include "FTPClientDataTypes.h"
#include "UnBlockingSocket\UnBlockingSocket.h"
#include "FTPClientFileState.h"


#ifndef INC_FTPCLIENT_H
#define INC_FTPCLIENT_H



enum  t_user_ftp_clt_mode
{
interactif,
quick_download,
quick_upload
} ;

typedef struct {
	char   name[36] ;
	unsigned long global_ipaddr;
	int    port;
	char   srv_login[36] ;
	char   srv_passwd[36] ;
	char   srv_startdir[36] ;
	t_user_ftp_clt_mode clt_mode;
	char   FileToDownload[36] ;
	char   FileToUpload[36];
	char   local_dir[36];
	unsigned int    feat; 
	unsigned int pasv;
	unsigned int zip;
} t_FtpCltPrm;

////////////////////////////////////////////////////////////////////////////////
/// Namespace for all FTP-related classes.
////////////////////////////////////////////////////////////////////////////////
namespace nsFTP
{

   using namespace nsSocket;

   class ITransferNotification
   {
   public:
	  virtual void OnBytesReceived(const char * /*vBuffer*/, long /*lReceivedBytes*/) {}
	  virtual void OnPreBytesSend(char * /*vBuffer*/, size_t& /*bytesToSend*/) {}
   };

   class CFile : public ITransferNotification
   {
      FILE* m_pFile;

   public:
      enum T_enOrigin { orBegin=SEEK_SET, orEnd=SEEK_END, orCurrent=SEEK_CUR };

      CFile();
      ~CFile();

      bool Open(const tstring* strFileName, const tstring& strMode);
      bool Close();
      bool Seek(u64 lOffset, T_enOrigin enOrigin);
      long Tell();
	  long Feof();

      size_t Write(const void* pBuffer, size_t itemSize, size_t itemCount);
      size_t Read(void* pBuffer, size_t itemSize, size_t itemCount);
      void OnBytesReceived(const char * vBuffer, long lReceivedBytes);
      void OnPreBytesSend(char * vBuffer, unsigned int vBufferSize, size_t& bytesToSend);
   };

   class CMemFile : public ITransferNotification
   {
      void* m_pFile;
	  u64   m_len;
	  u64   m_pos;
	  u64   m_final_pos;


   public:
      enum T_enOrigin { orBegin=SEEK_SET, orEnd=SEEK_END, orCurrent=SEEK_CUR };

      CMemFile();
      ~CMemFile();

	  bool Open(const void *ptr, u64 len);
      bool Close();
      u64 Size() {return m_final_pos;};

      bool Seek(u64 lOffset, T_enOrigin enOrigin);
      u64 Tell() ; 
	  u64 Feof();

      size_t Write(const void* pBuffer, size_t itemSize, size_t itemCount);
      size_t Read(void* pBuffer, size_t itemSize, size_t itemCount);
      void OnBytesReceived(const char * vBuffer, long lReceivedBytes);
      void OnPreBytesSend(char * vBuffer, unsigned int vBufferSize, size_t& bytesToSend);
   };

   class COutputStream : public ITransferNotification
   {
	  tstring mc_strEolCharacterSequence;
	  char *   m_vBufferStream;
	  unsigned long m_vBufferLength;
	  unsigned long m_nStart;

   public:
	  COutputStream(const tstring& strEolCharacterSequence) : mc_strEolCharacterSequence(strEolCharacterSequence) {};

	  void SetStartPosition();
	  bool toto(void);
	  bool GetNextLine(tstring& strLine);
	  bool GetLine(tstring& strLine) {strLine=m_vBufferStream; return true;}
	  void OnBytesReceived(char * vBuffer, long lReceivedBytes);
	  void OnPreBytesSend(char * vBuffer, size_t& bytesToSend){}

   };

   /// @brief FTP client class
   ///
   /// Use this class for all the ftp-client stuff such as
   /// - logon server
   /// - send and receive data
   /// - get directory listing
   /// - ...
   ///

   	extern "C" int errno;

   class CFTPClient
   {
   public:
   // @brief interface for notification
   ///
   /// Derive your class from this base-class and register this class on CFTPClient.
   /// For example you can use this for logging the sended and received commands.
   class CNotification 
   {
   public:


	nsHelper::CTimer  m_Timer;
	long    m_lTransferredBytes;
	long    m_lFileSize;


	void Reset()
	{
		  m_Timer.Restart();
		  m_lTransferredBytes = 0;
	   }

	void onStart(void)
	{
		sgIP_dbgprint("Start Observer\n");
	}


	void OnInternalError(const tstring& strFileName, DWORD dwLineNr)
   {
	  dbgprintclient_E("error: %d on %s line %d ",errno, strFileName.c_str(), dwLineNr );
   }
   
	void OnBeginReceivingData() 
   {
	   sgIP_dbgprint("Begin receive Data\n");
	  Reset(); 
   }

	void OnEndReceivingData(long lReceivedBytes) {
		dbgprintclient_R("Total received Data %d\n",lReceivedBytes);
	}
   
	void OnBytesReceived(const char * vBuffer, long lReceivedBytes)
   {
	  m_lTransferredBytes += lReceivedBytes;
	  //i9f( m_Timer.GetElapsedTime() >= 0.25 )
	  //{
		// sgIP_dbgprint("%d\n", static_cast<int>(static_cast<double>(m_lTransferredBytes)/m_lFileSize*100));
		// m_Timer.Restart();
	  //}
   }
   
	void OnBytesSent(const char * vBuffer, long lSentBytes)
   {
	  m_lTransferredBytes += lSentBytes;
	  //if( m_Timer.GetElapsedTime() >= 0.25 )
	  //{
		 //sgIP_dbgprint("%d\n",static_cast<int>(static_cast<double>(m_lTransferredBytes)/m_lFileSize*100));
		// m_Timer.Restart();
	  //}
   }

   
	void OnPreReceiveFile(const tstring* strSourceFile, const tstring* strTargetFile, long lFileSize) 
	{ 
		m_lFileSize = lFileSize; 
		dbgprintclient_R("Receive %s \n", strTargetFile->c_str()); //  strSourceFile->c_str() 
	}
	void OnPreSendFile(const tstring* strSourceFile, const tstring* strTargetFile, long lFileSize) 
	{
		dbgprintclient_S("Send src %s tgt %s \n", strSourceFile->c_str() , strTargetFile->c_str());

	};
	void OnPostReceiveFile(const tstring* strSourceFile, const tstring* strTargetFile, long lFileSize)
   {
		dbgprintclient_R("Receive src %s tgt %s\n", strSourceFile->c_str() , strTargetFile->c_str());
   }
	void OnPostSendFile(const tstring* strSourceFile, const tstring* strTargetFile, long lFileSize)
   {
		dbgprintclient_S("Send src %s tgt %s \n", strSourceFile->c_str() , strTargetFile->c_str() );
   }
	void OnSendCommand(const tstring* strCommand)
   {
	 dbgprintclient_R("%s\n", strCommand->c_str() );
   }
   
   virtual void OnResponse(const CReply& Reply)
   {
	   dbgprintclient_S(">>%s<<\n",Reply.m_strResponse.c_str());
   }
};

	  CFTPClient(unsigned int uiTimeout=10, unsigned int uiBufferSize=2048, unsigned int uiResponseWait=0);
	  virtual ~CFTPClient();

	  bool Init();
	  bool IsConnected();
	  bool IsTransferringData();
	  void SetResumeMode(bool fEnable=true);

	  bool Login(const CLogonInfo& loginInfo);
	  int  Logout();
	  const CLogonInfo& LastLogonInfo() { return m_LastLogonInfo; }

	  int List(const tstring* strPath,  bool fPasv=false, bool zip=false);
	  unsigned int NumberOfFiles(void) {return numberofFiles; }
	  
	  CFTPFileStatus*  GetFileStat(unsigned int FileNumber) {
		  if (FileNumber >mCFTPFileStatusNbr ||  FileNumber > numberofFiles)  return &DirList [0];
		  else return &DirList [FileNumber]; 
	  }

	  int NameList(const tstring* strPath, bool zip, bool fPasv, tstring& listOfFiles );
	  int  Delete(const tstring* strFile);
	  int  Rename(const tstring* strOldName, const tstring* strNewName);
	  int DownloadFile(const tstring* strRemoteFile, const tstring* strLocalFile, bool zip=true, bool overwrite=true,
						bool fPasv=false) ;
	  int DownloadMemFile(const tstring* strRemoteFile, const void* strLocalMemFile, 
								u32 strLocalMemFileLen, u32 *remoteFileLength, bool zip = true,
							 bool fPasv=false); 
	  int UploadFile(const tstring* strLocalFile, const tstring* strRemoteFile, bool zip=true, bool overwrite=true, bool fStoreUnique=false, 
					  bool fPasv=false) ;
	  int UploadMemFile(const void * strLocalFileMem, const u32 strLocalFileLength, const tstring* strRemoteFile, bool zip, bool overwrite, 
						   bool fStoreUnique =false, 
						   bool fPasv=false); 

	  int  RemoveDirectory(const tstring* strDirectory);
	  int  MakeDirectory(const tstring* strDirectory);
	  int PrintWorkingDirectory();
	  int ChangeToParentDirectory();
	  int ChangeWorkingDirectory(const tstring* strDirectory);
	  int Feat();
	  int OptionsModeZip( const unsigned int strLevel );
	  int Passive(unsigned long& ulIpAddress, unsigned int& ushPort);
	  int DataPort(const tstring* strHostIP, unsigned int ushPort);
	  int Abort();
	  int System();
	  int Noop();
	  int RepresentationType(const CRepresentation& repType, DWORD dwSize=0);
	  int FileStructure(const CStructure& crStructure);
	  int TransferMode(const CTransferMode& crTransferMode);
	  int Allocate(int iReserveBytes, const int* piMaxPageOrRecordSize=NULL);
	  int StructureMount(const tstring* strPath);
	  int SiteParameters(const tstring* strCmd);
	  int Status(const tstring* strPath);
	  int Help(const tstring* strTopic);
	  int Reinitialize();
	  int Restart(DWORD dwPosition);
	  int MaxNbFilesList (void) { return mCFTPFileStatusNbr ;}  
	  int FileSize(const tstring* strPath, u64& lSize);
	  int FileModificationTime(const tstring* strPath, tm& tmModificationTime);
	  int FileModificationTime(const tstring* strPath, tstring& strModificationTime);
	  unsigned int IsCursor(unsigned int curs)  { return curs<numberofFiles ? curs : (numberofFiles==0 ? 0 : numberofFiles) ; }; 
	  bool SerVerSupportSIZE(void) { return m_size_supported_by_server; }

   private:
	  enum t_Obserser {t_file, t_memfile, t_stream};
	  bool ExecuteDatachannelCommand(const CDatachannelCmd* crDatachannelCmd, const tstring* strPath ,
	const CRepresentation*representation, bool fPasv, bool overwrite, bool zip,	DWORD dwByteOffset,
	t_Obserser pObserver);


   private:
	  bool TransferData(const CDatachannelCmd* crDatachannelCmd, t_Obserser pObserver, bool zip);
	  bool SendData(t_Obserser pObserver) ;
	  bool SendDataZip(t_Obserser pObserver);
	  bool ReceiveData(t_Obserser pObserver) ;
	  bool ReceiveDataZip(t_Obserser pObserver) ;

	  tstring GetCmdString(const CDatachannelCmd* crDatachannelCmd, const tstring* strPath);
	  int  SimpleErrorCheck(CReply& Reply);

	  bool SendCommand(const tstring& strCommand) ;
	  bool SendCommand(const tstring& strCommand, CReply& Reply) ;
	  bool GetResponse(CReply& Reply) ;
	  bool GetSingleResponseLine(tstring& strResponse) ;
	  bool GetFeatMultipleResponseLine(tstring& strResponse) ;

	  bool OpenControlChannel(const tstring* strServerHost, unsigned int ushServerPort=DEFAULT_FTP_PORT);
	  void CloseControlChannel();

	  void ReportError(const tstring& strFile, DWORD dwLineNr) ;
	  bool GetIpAddressFromResponse(const tstring* strResponse, unsigned long& ulIpAddress, unsigned int& ushPort);

// data members
   private:
	  enum T_enTransferMode { tmStream, tmBlock, tmCompressed };
	  const unsigned int                     mc_uiTimeout;               ///< timeout for socket-functions
	  const unsigned int                     mc_uiResponseWait;          ///< sleep time between receive calls to socket when getting the response
	  const tstring                          mc_strEolCharacterSequence; ///< end-of-line sequence of current operating system

	  char      							 *m_vBuffer;                 ///< buffer for sending and receiving
	  const unsigned int					 m_vBufferSize;
	  CRepresentation						 m_apCurrentRepresentation;  ///< representation currently set
	  T_enTransferMode						 m_apCurrentTransfertMode;   /// Current Transfert Mode
	  CUnBlockingSocket				         m_apSckControlConnection;   ///< socket for connection to ftp-server
	  CUnBlockingSocket				         m_sckDataConnection;          /// Socket data connection
	  bool									 m_fTransferInProgress;      ///< if true, a file transfer is in progress
	  bool									 m_fAbortTransfer;           ///< indicates that a running filetransfer should be canceled
	  bool                                   m_fResumeIfPossible;        ///< try to resume download/upload if possible
	  CLogonInfo                             m_LastLogonInfo;            ///< logon-info, which was used at the last call of login
	  ITransferNotification					 pObserver;
	  CNotification							 m_notification;			 /// Notify all actions
	  COutputStream							 m_coutputstream; 
	  CFile									 cfile;
	  CMemFile								 cmemfile;
	  int									 iZlibLevel;
	  unsigned int							 numberofFiles;
	  CFTPFileStatus						 DirList [200] ;
	  const unsigned int                     mCFTPFileStatusNbr;

	  bool                                  m_zip_supported_by_server;
	  bool									m_size_supported_by_server;
	  bool									m_zip;
	  bool									m_feat;

   };

}

#endif // INC_FTPCLIENT_H

// FTP commands - Overview

// simple commands
//   CDUP <CRLF>
//   QUIT <CRLF>
//   REIN <CRLF>
//   PASV <CRLF>
//   STOU <CRLF>
//   ABOR <CRLF>
//   PWD  <CRLF>
//   SYST <CRLF>
//   NOOP <CRLF>
//   PORT <SP> <host-port> <CRLF>
//   TYPE <SP> <type-code> <CRLF>
//   CWD  <SP> <pathname> <CRLF>
//   MKD  <SP> <pathname> <CRLF>
//   SITE <SP> <string> <CRLF>
//   HELP [<SP> <string>] <CRLF>
//   DELE <SP> <pathname> <CRLF>
//   RMD  <SP> <pathname> <CRLF>
//   STRU <SP> <structure-code> <CRLF>
//   MODE <SP> <mode-code> <CRLF>
//   STAT [<SP> <pathname>] <CRLF>
//   ALLO <SP> <decimal-integer>
//       [<SP> R <SP> <decimal-integer>] <CRLF>
//   SMNT <SP> <pathname> <CRLF>

// commands for logon sequence
//   USER <SP> <username> <CRLF>
//   PASS <SP> <password> <CRLF>
//   ACCT <SP> <account-information> <CRLF>

// commands for renaming
//   RNFR <SP> <pathname> <CRLF>
//   RNTO <SP> <pathname> <CRLF>

//   RETR <SP> <pathname> <CRLF>
//   STOR <SP> <pathname> <CRLF>
//   APPE <SP> <pathname> <CRLF>
//   REST <SP> <marker> <CRLF>
//   LIST [<SP> <pathname>] <CRLF>
//   NLST [<SP> <pathname>] <CRLF>

// non RFC-Commands
//   SIZE <SP> <pathname> <CRLF>
//   MDTM <SP> <pathname> <CRLF>

/** \class nsFTP::CStructure
   In addition to different representation types, FTP allows the
   structure of a file to be specified.  Three file structures are
   defined in FTP:

	  - file-structure,     where there is no internal structure and
						   the file is considered to be a
						   continuous sequence of data bytes,

	  - record-structure,   where the file is made up of sequential
						   records,

	  - and page-structure, where the file is made up of independent
						   indexed pages.

   File-structure is the default to be assumed if the STRUcture
   command has not been used but both file and record structures
   must be accepted for "text" files (i.e., files with TYPE ASCII
   or EBCDIC) by all FTP implementations.  The structure of a file
   will affect both the transfer mode of a file (see the Section
   on Transmission Modes) and the interpretation and storage of
   the file.

   The "natural" structure of a file will depend on which host
   stores the file.  A source-code file will usually be stored on
   an IBM Mainframe in fixed length records but on a DEC TOPS-20
   as a stream of characters partitioned into lines, for example
   by <CRLF>.  If the transfer of files between such disparate
   sites is to be useful, there must be some way for one site to
   recognize the other's assumptions about the file.

   With some sites being naturally file-oriented and others
   naturally record-oriented there may be problems if a file with
   one structure is sent to a host oriented to the other.  If a
   text file is sent with record-structure to a host which is file
   oriented, then that host should apply an internal
   transformation to the file based on the record structure.
   Obviously, this transformation should be useful, but it must
   also be invertible so that an identical file may be retrieved
   using record structure.

   In the case of a file being sent with file-structure to a
   record-oriented host, there exists the question of what
   criteria the host should use to divide the file into records
   which can be processed locally.  If this division is necessary,
   the FTP implementation should use the end-of-line sequence,

   <CRLF> for ASCII, or <NL> for EBCDIC text files, as the
   delimiter.  If an FTP implementation adopts this technique, it
   must be prepared to reverse the transformation if the file is
   retrieved with file-structure.
*/

/** \fn static const CStructure nsFTP::CStructure::File()
   File structure is the default to be assumed if the STRUcture
   command has not been used.
   In file-structure there is no internal structure and the
   file is considered to be a continuous sequence of data
   bytes.
*/
/** \fn static const CStructure nsFTP::CStructure::Record()
   Record structures must be accepted for "text" files (i.e.,
   files with TYPE ASCII or EBCDIC) by all FTP implementations.
   In record-structure the file is made up of sequential
   records.
*/
/** \fn static const CStructure nsFTP::CStructure::Page()
   To transmit files that are discontinuous, FTP defines a page
   structure.  Files of this type are sometimes known as
   "random access files" or even as "holey files".  In these
   files there is sometimes other information associated with
   the file as a whole (e.g., a file descriptor), or with a
   section of the file (e.g., page access controls), or both.
   In FTP, the sections of the file are called pages.

   To provide for various page sizes and associated
   information, each page is sent with a page header.  The page
   header has the following defined fields:

	  - Header Length\n
		 The number of logical bytes in the page header
		 including this byte.  The minimum header length is 4.

	  - Page Index\n
		 The logical page number of this section of the file.
		 This is not the transmission sequence number of this
		 page, but the index used to identify this page of the
		 file.

	  - Data Length\n
		 The number of logical bytes in the page data.  The
		 minimum data length is 0.

	  - Page Type\n
		 The type of page this is.  The following page types
		 are defined:

<PRE>         0 = Last Page
			This is used to indicate the end of a paged
			structured transmission.  The header length must
			be 4, and the data length must be 0.

		 1 = Simple Page
			This is the normal type for simple paged files
			with no page level associated control
			information.  The header length must be 4.

		 2 = Descriptor Page
			This type is used to transmit the descriptive
			information for the file as a whole.

		 3 = Access Controlled Page
			This type includes an additional header field
			for paged files with page level access control
			information.  The header length must be 5.</PRE>

	  - Optional Fields\n
		 Further header fields may be used to supply per page
		 control information, for example, per page access
		 control.

   All fields are one logical byte in length.  The logical byte
   size is specified by the TYPE command.  See Appendix I for
   further details and a specific case at the page structure.

   A note of caution about parameters:  a file must be stored and
   retrieved with the same parameters if the retrieved version is to

   be identical to the version originally transmitted.  Conversely,
   FTP implementations must return a file identical to the original
   if the parameters used to store and retrieve a file are the same.
*/

/** \class nsFTP::CTransferMode
   The next consideration in transferring data is choosing the
   appropriate transmission mode.  There are three modes: one which
   formats the data and allows for restart procedures; one which also
   compresses the data for efficient transfer; and one which passes
   the data with little or no processing.  In this last case the mode
   interacts with the structure attribute to determine the type of
   processing.  In the compressed mode, the representation type
   determines the filler byte.

   All data transfers must be completed with an end-of-file (EOF)
   which may be explicitly stated or implied by the closing of the
   data connection.  For files with record structure, all the
   end-of-record markers (EOR) are explicit, including the final one.
   For files transmitted in page structure a "last-page" page type is
   used.

   NOTE:  In the rest of this section, byte means "transfer byte"
   except where explicitly stated otherwise.

   For the purpose of standardized transfer, the sending host will
   translate its internal end of line or end of record denotation
   into the representation prescribed by the transfer mode and file
   structure, and the receiving host will perform the inverse
   translation to its internal denotation.  An IBM Mainframe record
   count field may not be recognized at another host, so the
   end-of-record information may be transferred as a two byte control
   code in Stream mode or as a flagged bit in a Block or Compressed
   mode descriptor.  End-of-line in an ASCII or EBCDIC file with no
   record structure should be indicated by <CRLF> or <NL>,
   respectively.  Since these transformations imply extra work for
   some systems, identical systems transferring non-record structured
   text files might wish to use a binary representation and stream
   mode for the transfer.
*/

/** \fn static const CTransferMode nsFTP::CTransferMode::Stream()
   The data is transmitted as a stream of bytes.  There is no
   restriction on the representation type used; record structures
   are allowed.

   In a record structured file EOR and EOF will each be indicated
   by a two-byte control code.  The first byte of the control code
   will be all ones, the escape character.  The second byte will
   have the low order bit on and zeros elsewhere for EOR and the
   second low order bit on for EOF; that is, the byte will have
   value 1 for EOR and value 2 for EOF.  EOR and EOF may be
   indicated together on the last byte transmitted by turning both
   low order bits on (i.e., the value 3).  If a byte of all ones
   was intended to be sent as data, it should be repeated in the
   second byte of the control code.

   If the structure is a file structure, the EOF is indicated by
   the sending host closing the data connection and all bytes are
   data bytes.
*/

/** \fn static const CTransferMode nsFTP::CTransferMode::Block()
   The file is transmitted as a series of data blocks preceded by
   one or more header bytes.  The header bytes contain a count
   field, and descriptor code.  The count field indicates the
   total length of the data block in bytes, thus marking the
   beginning of the next data block (there are no filler bits).
   The descriptor code defines:  last block in the file (EOF) last
   block in the record (EOR), restart marker (see the Section on
   Error Recovery and Restart) or suspect data (i.e., the data
   being transferred is suspected of errors and is not reliable).
   This last code is NOT intended for error control within FTP.
   It is motivated by the desire of sites exchanging certain types
   of data (e.g., seismic or weather data) to send and receive all
   the data despite local errors (such as "magnetic tape read
   errors"), but to indicate in the transmission that certain
   portions are suspect).  Record structures are allowed in this
   mode, and any representation type may be used.

   The header consists of the three bytes.  Of the 24 bits of
   header information, the 16 low order bits shall represent byte
   count, and the 8 high order bits shall represent descriptor
   codes as shown below.

   Block Header
<PRE>
	  +----------------+----------------+----------------+
	  | Descriptor     |    Byte Count                   |
	  |         8 bits |                      16 bits    |
	  +----------------+----------------+----------------+
</PRE>
   The descriptor codes are indicated by bit flags in the
   descriptor byte.  Four codes have been assigned, where each
   code number is the decimal value of the corresponding bit in
   the byte.
<PRE>
	  Code     Meaning

		128     End of data block is EOR
		 64     End of data block is EOF
		 32     Suspected errors in data block
		 16     Data block is a restart marker
</PRE>
   With this encoding, more than one descriptor coded condition
   may exist for a particular block.  As many bits as necessary
   may be flagged.

   The restart marker is embedded in the data stream as an
   integral number of 8-bit bytes representing printable
   characters in the language being used over the control
   connection (e.g., default--NVT-ASCII).  <SP> (Space, in the
   appropriate language) must not be used WITHIN a restart marker.

   For example, to transmit a six-character marker, the following
   would be sent:
<PRE>
	  +--------+--------+--------+
	  |Descrptr|  Byte count     |
	  |code= 16|             = 6 |
	  +--------+--------+--------+

	  +--------+--------+--------+
	  | Marker | Marker | Marker |
	  | 8 bits | 8 bits | 8 bits |
	  +--------+--------+--------+

	  +--------+--------+--------+
	  | Marker | Marker | Marker |
	  | 8 bits | 8 bits | 8 bits |
	  +--------+--------+--------+
</PRE>
*/

/** \fn static const CTransferMode nsFTP::CTransferMode::Compressed()
   There are three kinds of information to be sent:  regular data,
   sent in a byte string; compressed data, consisting of
   replications or filler; and control information, sent in a
   two-byte escape sequence.  If n>0 bytes (up to 127) of regular
   data are sent, these n bytes are preceded by a byte with the
   left-most bit set to 0 and the right-most 7 bits containing the
   number n.

   Byte string:
<PRE>
		 1       7                8                     8
	  +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+     +-+-+-+-+-+-+-+-+
	  |0|       n     | |    d(1)       | ... |      d(n)     |
	  +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+     +-+-+-+-+-+-+-+-+
									^             ^
									|---n bytes---|
										  of data
</PRE>
   String of n data bytes d(1),..., d(n)
   Count n must be positive.

   To compress a string of n replications of the data byte d, the
   following 2 bytes are sent:

   Replicated Byte:
<PRE>
		 2       6               8
	  +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+
	  |1 0|     n     | |       d       |
	  +-+-+-+-+-+-+-+-+ +-+-+-+-+-+-+-+-+
</PRE>
   A string of n filler bytes can be compressed into a single
   byte, where the filler byte varies with the representation
   type.  If the type is ASCII or EBCDIC the filler byte is <SP>
   (Space, ASCII code 32, EBCDIC code 64).  If the type is Image
   or Local byte the filler is a zero byte.

   Filler String:
<PRE>
		 2       6
	  +-+-+-+-+-+-+-+-+
	  |1 1|     n     |
	  +-+-+-+-+-+-+-+-+
</PRE>
   The escape sequence is a double byte, the first of which is the
   escape byte (all zeros) and the second of which contains
   descriptor codes as defined in Block mode.  The descriptor
   codes have the same meaning as in Block mode and apply to the
   succeeding string of bytes.

   Compressed mode is useful for obtaining increased bandwidth on
   very large network transmissions at a little extra CPU cost.
   It can be most effectively used to reduce the size of printer
   files such as those generated by RJE hosts.
*/

/** \class nsFTP::CRepresentation
   DATA REPRESENTATION AND STORAGE

   Data is transferred from a storage device in the sending host to a
   storage device in the receiving host.  Often it is necessary to
   perform certain transformations on the data because data storage
   representations in the two systems are different.  For example,
   NVT-ASCII has different data storage representations in different
   systems.  DEC TOPS-20s's generally store NVT-ASCII as five 7-bit
   ASCII characters, left-justified in a 36-bit word. IBM Mainframe's
   store NVT-ASCII as 8-bit EBCDIC codes.  Multics stores NVT-ASCII
   as four 9-bit characters in a 36-bit word.  It is desirable to
   convert characters into the standard NVT-ASCII representation when
   transmitting text between dissimilar systems.  The sending and
   receiving sites would have to perform the necessary
   transformations between the standard representation and their
   internal representations.

   A different problem in representation arises when transmitting
   binary data (not character codes) between host systems with
   different word lengths.  It is not always clear how the sender
   should send data, and the receiver store it.  For example, when
   transmitting 32-bit bytes from a 32-bit word-length system to a
   36-bit word-length system, it may be desirable (for reasons of
   efficiency and usefulness) to store the 32-bit bytes
   right-justified in a 36-bit word in the latter system.  In any
   case, the user should have the option of specifying data
   representation and transformation functions.  It should be noted

   that FTP provides for very limited data type representations.
   Transformations desired beyond this limited capability should be
   performed by the user directly.

   Several types take a second parameter. The first parameter is
   denoted by a single Telnet character, as is the second
   Format parameter for ASCII and EBCDIC; the second parameter
   for local byte is a decimal integer to indicate Bytesize.
   The parameters are separated by a <SP> (Space, ASCII code
   32).

   The following codes are assigned for type:
<PRE>
				\    /
	  A - ASCII |    | N - Non-print
				|-><-| T - Telnet format effectors
	  E - EBCDIC|    | C - Carriage Control (ASA)
				/    \
	  I - Image

	  L <byte size> - Local byte Byte size
</PRE>
   The default representation type is ASCII Non-print.  If the
   Format parameter is changed, and later just the first
   argument is changed, Format then returns to the Non-print
   default.
*/

/** \class nsFTP::CType
   Objects of this class are only used in conjunction with CRepresentation.

   DATA TYPES

   Data representations are handled in FTP by a user specifying a
   representation type.  This type may implicitly (as in ASCII or
   EBCDIC) or explicitly (as in Local byte) define a byte size for
   interpretation which is referred to as the "logical byte size."
   Note that this has nothing to do with the byte size used for
   transmission over the data connection, called the "transfer
   byte size", and the two should not be confused.  For example,
   NVT-ASCII has a logical byte size of 8 bits.  If the type is
   Local byte, then the TYPE command has an obligatory second
   parameter specifying the logical byte size.  The transfer byte
   size is always 8 bits.
*/

/** \fn static const CType nsFTP::CType::ASCII()
   This is the default type and must be accepted by all FTP
   implementations.  It is intended primarily for the transfer
   of text files, except when both hosts would find the EBCDIC
   type more convenient.

   The sender converts the data from an internal character
   representation to the standard 8-bit NVT-ASCII
   representation (see the Telnet specification).  The receiver
   will convert the data from the standard form to his own
   internal form.

   In accordance with the NVT standard, the <CRLF> sequence
   should be used where necessary to denote the end of a line
   of text.  (See the discussion of file structure at the end
   of the Section on Data Representation and Storage.)

   Using the standard NVT-ASCII representation means that data
   must be interpreted as 8-bit bytes.

   The Format parameter for ASCII and EBCDIC types is discussed
   below.
*/

/** \fn static const CType nsFTP::CType::EBCDIC()
   This type is intended for efficient transfer between hosts
   which use EBCDIC for their internal character
   representation.

   For transmission, the data are represented as 8-bit EBCDIC
   characters.  The character code is the only difference
   between the functional specifications of EBCDIC and ASCII
   types.

   End-of-line (as opposed to end-of-record--see the discussion
   of structure) will probably be rarely used with EBCDIC type
   for purposes of denoting structure, but where it is
   necessary the <NL> character should be used.
*/

/** \fn static const CType nsFTP::CType::Image()
   The data are sent as contiguous bits which, for transfer,
   are packed into the 8-bit transfer bytes.  The receiving
   site must store the data as contiguous bits.  The structure
   of the storage system might necessitate the padding of the
   file (or of each record, for a record-structured file) to
   some convenient boundary (byte, word or block).  This
   padding, which must be all zeros, may occur only at the end
   of the file (or at the end of each record) and there must be
   a way of identifying the padding bits so that they may be
   stripped off if the file is retrieved.  The padding
   transformation should be well publicized to enable a user to
   process a file at the storage site.

   Image type is intended for the efficient storage and
   retrieval of files and for the transfer of binary data.  It
   is recommended that this type be accepted by all FTP
   implementations.
*/

/** \fn static const CType nsFTP::CType::LocalByte()
   The data is transferred in logical bytes of the size
   specified by the obligatory second parameter, Byte size.
   The value of Byte size must be a decimal integer; there is
   no default value.  The logical byte size is not necessarily
   the same as the transfer byte size.  If there is a
   difference in byte sizes, then the logical bytes should be
   packed contiguously, disregarding transfer byte boundaries
   and with any necessary padding at the end.

   When the data reaches the receiving host, it will be
   transformed in a manner dependent on the logical byte size
   and the particular host.  This transformation must be
   invertible (i.e., an identical file can be retrieved if the
   same parameters are used) and should be well publicized by
   the FTP implementors.

   For example, a user sending 36-bit floating-point numbers to
   a host with a 32-bit word could send that data as Local byte
   with a logical byte size of 36.  The receiving host would
   then be expected to store the logical bytes so that they
   could be easily manipulated; in this example putting the
   36-bit logical bytes into 64-bit double words should
   suffice.

   In another example, a pair of hosts with a 36-bit word size
   may send data to one another in words by using TYPE L 36.
   The data would be sent in the 8-bit transmission bytes
   packed so that 9 transmission bytes carried two host words.
*/

/** \class nsFTP::CTypeFormat
   Objects of this class are only used in conjunction with CRepresentation.

   FORMAT CONTROL

   The types ASCII and EBCDIC also take a second (optional)
   parameter; this is to indicate what kind of vertical format
   control, if any, is associated with a file.  The following
   data representation types are defined in FTP:

   A character file may be transferred to a host for one of
   three purposes: for printing, for storage and later
   retrieval, or for processing.  If a file is sent for
   printing, the receiving host must know how the vertical
   format control is represented.  In the second case, it must
   be possible to store a file at a host and then retrieve it
   later in exactly the same form.  Finally, it should be
   possible to move a file from one host to another and process
   the file at the second host without undue trouble.  A single
   ASCII or EBCDIC format does not satisfy all these
   conditions.  Therefore, these types have a second parameter
   specifying one of the following three formats:
*/

/** \fn static const CTypeFormat nsFTP::CTypeFormat::NonPrint()
   NON PRINT

   This is the default format to be used if the second
   (format) parameter is omitted.  Non-print format must be
   accepted by all FTP implementations.

   The file need contain no vertical format information.  If
   it is passed to a printer process, this process may
   assume standard values for spacing and margins.

   Normally, this format will be used with files destined
   for processing or just storage.
*/

/** \fn static const CTypeFormat nsFTP::CTypeFormat::TelnetFormat()
   TELNET FORMAT CONTROLS

   The file contains ASCII/EBCDIC vertical format controls
   (i.e., <CR>, <LF>, <NL>, <VT>, <FF>) which the printer
   process will interpret appropriately.  <CRLF>, in exactly
   this sequence, also denotes end-of-line.
*/

/** \fn static const CTypeFormat nsFTP::CTypeFormat::CarriageControl()
   CARRIAGE CONTROL (ASA)

   The file contains ASA (FORTRAN) vertical format control
   characters.  (See RFC 740 Appendix C; and Communications
   of the ACM, Vol. 7, No. 10, p. 606, October 1964.)  In a
   line or a record formatted according to the ASA Standard,
   the first character is not to be printed.  Instead, it
   should be used to determine the vertical movement of the
   paper which should take place before the rest of the
   record is printed.

   The ASA Standard specifies the following control
   characters:

	  Character     Vertical Spacing

	  blank         Move paper up one line
	  0             Move paper up two lines
	  1             Move paper to top of next page
	  +             No movement, i.e., overprint

   Clearly there must be some way for a printer process to
   distinguish the end of the structural entity.  If a file
   has record structure (see below) this is no problem;
   records will be explicitly marked during transfer and
   storage.  If the file has no record structure, the <CRLF>
   end-of-line sequence is used to separate printing lines,
   but these format effectors are overridden by the ASA
   controls.
*/

