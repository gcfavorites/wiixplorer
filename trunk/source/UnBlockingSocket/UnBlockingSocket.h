/****************************************************************************
 * Copyright (C) 2009
 * by LilouMaster based oFrom David J. Kruglinski 
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

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory>
#include <network.h>
#include <zlib.h>

#include "UnBlockingSocketDefinements.h"
//#include "FTPClientconfig.h"

#ifndef INC_BLOCKINGSOCKET_H
#define INC_BLOCKINGSOCKET_H

#define SOCKET_UNDEF (-1)

namespace nsSocket
{

typedef const struct sockaddr* LPCSOCKADDR;

///////////////////////////////////////////////////////////////////////////////////////
//***************************** CUnBlockingSocketException  ***************************//
///////////////////////////////////////////////////////////////////////////////////////
class CUnBlockingSocketException
{
public:
   // Constructor
   CUnBlockingSocketException(LPTSTR pchMessage);
   
public:
   ~CUnBlockingSocketException() {}
   virtual BOOL GetErrorMessage( LPTSTR Error, UINT nMaxError, PUINT pnHelpContext = NULL);
   virtual tstring GetErrorMessage(PUINT pnHelpContext = NULL);
   
private:
   s32     m_nError;
   tstring m_strMessage;
};

///////////////////////////////////////////////////////////////////////////////////////
//*********************************** CSockAddr  ************************************//
///////////////////////////////////////////////////////////////////////////////////////
class CSockAddr : public sockaddr_in
{
public:
   // constructors
   CSockAddr();
   CSockAddr(const SOCKADDR& sa);
   CSockAddr(const SOCKADDR_IN& sin);
   CSockAddr(unsigned long ulAddr, u32 ushPort = 0); // parms are host byte ordered
   CSockAddr(const char* pchIP, u32 ushPort = 0) ;
   
   // Return the address in dotted-decimal format
   tstring DottedDecimal();

   // Get port and address (even though they're public)
   u32 Port() const ;
   unsigned long IPADDR() const;
   
   // operators added for efficiency
   CSockAddr& operator=(const SOCKADDR& sa)
            {
               memcpy(this, &sa, sizeof(SOCKADDR));
               return *this;
            }

   CSockAddr& operator=(const SOCKADDR_IN& sin)
            { 
               memcpy(this, &sin, sizeof(SOCKADDR_IN));
               return *this; 
            }

   operator SOCKADDR()
            { return *((LPSOCKADDR) this); }

   operator LPSOCKADDR()
            { return (LPSOCKADDR) this; }

   operator LPSOCKADDR_IN()
            { return (LPSOCKADDR_IN) this; }
};

///////////////////////////////////////////////////////////////////////////////////////
//********************************* CUnBlockingSocket  ********************************//
///////////////////////////////////////////////////////////////////////////////////////

class CUnBlockingSocket 
{
public :
	/// Enum the differents Modes of Transfer.
	enum DataMode_t {
		STREAM,
		ZLIB
	} ;

	/// Enum the differents Type of Transfer.
	enum DataType_t {
		ASCII,
		BINARY, ///< equals to IMAGE
		EBCDIC
	} ;

	/// Enum the different Status 
	enum Status_t {
		WAITING,
		LISTING,
		UPLOADING,
		DOWNLOADING
	}  ;

   struct DataTransfer_t
		{
		s32 iZlibLevel;
		z_stream zStream;

		DataMode_t eDataMode;
		DataType_t eDataType;

		} CurrentTransfer;

		bool InitZlib( DataTransfer_t *pTransfer , Status_t eStatus );

public:

	enum SockType_t {
		ApplicationSocket,
		DataSocket
	}  eSockType;
      

	  
   CUnBlockingSocket() :
      m_hSocket(-1) {}

   void Init(SockType_t);
   void Cleanup();
   bool Create(s32 nType = SOCK_STREAM);
   bool Close();

   bool Bind(LPCSOCKADDR psa) const;
   bool Listen() const;
   bool Connect(LPCSOCKADDR psa) ;
   bool Accept() ;
   u32  Send(const char* pch, u32 nSize, s32 nSecs) const;
   u32  Write(const char* pch, u32 nSize, s32 nSecs) const;
   u32  ReceiveLine(char* pch, u32 nSize, s32 nSecs, boolean  exitonempty = true) const;
   u32  Receive(char* pch, u32 nSize, s32 nSecs, boolean  exitonempty = true) const;
   s32  SendDatagram(const char* pch, s32 nSize, LPCSOCKADDR psa, s32 nSecs) const;
   s32  ReceiveDatagram(char* pch, s32 nSize, LPSOCKADDR psa, s32 nSecs) const;
   bool GetPeerAddr(LPSOCKADDR psa) const;
   bool GetSockAddr(LPSOCKADDR psa) const;
   bool CheckReadability() const;
   s32 set_blocking(SOCKET s, bool blocking) const;
   s32 close_blocking(SOCKET s) const;
   bool Isconnected() {return m_connected; } ;

   CSockAddr   GetHostByName(const char* pchName, u32 ushPort = 0);
   const char* GetHostByAddr(LPCSOCKADDR psa);
   unsigned long GetIPAddr()  const { return IPAddr; }

   operator SOCKET() const { return m_hSocket; }
private:
   SOCKET m_hSocket;
   SOCKET m_aSocket;
   unsigned long int IPAddr;
   bool   m_connected;
};

CUnBlockingSocket* CreateDefaultBlockingSocketInstance();


};

#endif // INC_BLOCKINGSOCKET_H
