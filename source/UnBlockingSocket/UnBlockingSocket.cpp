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


// CUnBlockingSocketException, CUnBlockingSocket, CHttpBlockingSocket
#include <algorithm>
#include "UnBlockingSocket.h"
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
#include <network.h>
#define NET_TIMEOUT  10000  // network connection timeout, in ms

extern  struct hostent * gethostbyname(const char * name);
extern unsigned long ConvertIPAddress(const char * ipaddr_buf ) ;

unsigned long ConvertIPAddress(const char * ipaddr_buf ) 
{
	s32 i,j;
	u32 calculatedip;
	i=0;j=0;

	calculatedip =0;
	while(j<4) {
		if(*ipaddr_buf=='.' || *ipaddr_buf==0) {
			calculatedip = (calculatedip >> 8) | (i<<24);
			i=0;
			j++;
			if(*ipaddr_buf==0 && j<4) {
				calculatedip = calculatedip >> ((4-j)*8);
				j=4;
			}
		} else {
			if(*ipaddr_buf>='0' && *ipaddr_buf<='9') {
				i=i*10+(*ipaddr_buf-'0');
			}
		}
		ipaddr_buf++;
	}
	return calculatedip;
}


extern "C" unsigned long Wifi_GetIP();
extern "C" {
	extern  void waitpersecond ( u32 nbr );
};
extern s32 transmit_err;
extern void waitkey(void);
extern void waitkey(void);

using namespace nsSocket;

   // constructors
CSockAddr::CSockAddr()
   {
      sin_family        = AF_INET; // specifies address family
      sin_port          = 0;       // ip port
      sin_addr.s_addr   = 0;       // ip address
   }
   
CSockAddr::CSockAddr(const SOCKADDR& sa)
            { memcpy(this, &sa, sizeof(SOCKADDR)); }
   
CSockAddr::CSockAddr(const SOCKADDR_IN& sin) 
            { memcpy(this, &sin, sizeof(SOCKADDR_IN)); }
   
CSockAddr::CSockAddr(unsigned long ulAddr, u32 ushPort ) // parms are host byte ordered
            { 
               sin_family      = AF_INET;          // specifies address family
               sin_port        = htons(ushPort);   // ip port
               sin_addr.s_addr = htonl(ulAddr);    // ip address
            }

CSockAddr::CSockAddr(const char* pchIP, u32 ushPort) // dotted IP addr string
            { 
               sin_family = AF_INET;               // specifies address family
               sin_port = htons(ushPort);          // ip port
               sin_addr.s_addr = inet_addr(pchIP); // ip address
            } // already network byte ordered
   




   // Return the address in dotted-decimal format
tstring CSockAddr::DottedDecimal()
      { 
	char buffer[16];
	tstring ret;

	buffer[0]=0;
	u32 my_ip = net_gethostip();
	struct in_addr addr;
	addr.s_addr =my_ip;
		
	strcpy(buffer,inet_ntoa(addr));
	
		ret=buffer;
		return ret;
}

   // Get port and address (even though they're public)
   u32 CSockAddr::Port() const
            { return ntohs(sin_port); }
   
   unsigned long CSockAddr::IPADDR() const
            { return ntohl(sin_addr.s_addr); }
   






///////////////////////////////////////////////////////////////////////////////////////
//************************* Class CUnBlockingSocketException **************************//
///////////////////////////////////////////////////////////////////////////////////////

CUnBlockingSocketException::CUnBlockingSocketException(LPTSTR pchMessage)
{
   m_strMessage = pchMessage;
   m_nError     = WSAGetLastError();
}

BOOL CUnBlockingSocketException::GetErrorMessage( LPTSTR Error, UINT nMaxError,
      PUINT /*pnHelpContext = NULL*/)
{
	
   if( m_nError == 0 )
      sprintf(Error, "%s error", m_strMessage.c_str());
   else
      sprintf(Error,"%s error 0x%08x", m_strMessage.c_str(), m_nError);
	 
   return TRUE;
}

tstring CUnBlockingSocketException::GetErrorMessage(PUINT /*pnHelpContext = NULL*/)
{
   TCHAR szBuffer[512] = {0};
   GetErrorMessage(szBuffer, 512);

   return szBuffer;
}

bool CUnBlockingSocket::InitZlib( DataTransfer_t *pTransfer , Status_t eStatus )
{
	s32 nRet=Z_OK;
	pTransfer->zStream.zfree = Z_NULL;
	pTransfer->zStream.zalloc = Z_NULL;
	pTransfer->zStream.opaque = Z_NULL;

	switch( eStatus ) { // status of the client 
		case LISTING:
		case DOWNLOADING:
			nRet = deflateInit( &pTransfer->zStream, pTransfer->iZlibLevel ); // compresser 
			if( nRet == Z_OK )
				return true;
			break;
		case UPLOADING:
			nRet = inflateInit( &pTransfer->zStream ); // decompress
			if( nRet == Z_OK )
				return true;
			break;
		case WAITING:
			break;
	}

	switch( nRet ) {
		case Z_MEM_ERROR:sgIP_dbgprint("deflateInit %d\n",__LINE__);
			break;
		case Z_VERSION_ERROR:sgIP_dbgprint("deflateInit %d\n",__LINE__);
			break;
		case Z_STREAM_ERROR:sgIP_dbgprint("deflateInit %d\n",__LINE__);
			break;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////
//******************************** Class CUnBlockingSocket ****************************//
///////////////////////////////////////////////////////////////////////////////////////

void CUnBlockingSocket::Init(SockType_t SockType)
{
	m_hSocket=SOCKET_UNDEF;
	m_aSocket=SOCKET_UNDEF;
	m_connected=false;
	eSockType = SockType;
	IPAddr=0;
}


void CUnBlockingSocket::Cleanup()
{
	m_connected=false;

	s32 cr;
	s32 oldcr=-1;
		
   if( m_hSocket==SOCKET_UNDEF ) 
      return ;

   {
 	time_t start = time(NULL);
	time_t end = time(NULL);
    time_t time_now = 0;
	do
    {
		cr = net_close(m_hSocket);
        time_now = time(NULL);
		if (cr != oldcr) {oldcr=cr;sgIP_dbgprint("close %d ",cr);}

    } while((cr == -EAGAIN && time_now - start < 5) || time_now - end > 10); // try 5 second
   }

  if(m_aSocket !=  m_hSocket)
  {
	time_t start = time(NULL);
	time_t end = time(NULL);
    time_t time_now = 0;
	do
    {
		cr = net_close(m_aSocket);
       time_now = time(NULL);
    } while((cr == -EAGAIN && time_now - start < 5) || time_now - end > 10); // try 5 second
  }
   
   m_hSocket = SOCKET_UNDEF;
   m_aSocket = SOCKET_UNDEF;
  
 }


bool CUnBlockingSocket::Close()
{

   Cleanup();
   return true;
}

s32 CUnBlockingSocket::set_blocking(SOCKET s, bool blocking) const { 
   s32 flags; 
     flags = net_fcntl(s, F_GETFL, 0); 
     if (flags >= 0) flags = net_fcntl(s, F_SETFL, blocking ? (flags&~4) : (flags|4)); 
     return flags; 
 } 
  
s32 CUnBlockingSocket::close_blocking(SOCKET s) const { 
     set_blocking(s, true); 
     return net_close(s); 
 } 
 

bool CUnBlockingSocket::Create(s32 nType /* = SOCK_STREAM */)
{
   ASSERT( m_hSocket==SOCKET_UNDEF );

   if( (m_hSocket=net_socket(AF_INET, nType, IPPROTO_IP))<0 ) 
   {
      return false;
   }
   
   return true;
}

bool CUnBlockingSocket::Bind(LPCSOCKADDR psa) const
{
	s32 cr ;


   ASSERT( m_hSocket!=SOCKET_UNDEF );

   set_blocking( m_hSocket, false );


   if( (cr=net_bind(m_hSocket, (sockaddr *)psa, sizeof(SOCKADDR))) <0 )
   {
    if ( cr == -EADDRINUSE) return true;

	sgIP_dbgprint("Bind Ko %d\n",cr);
 	  
      return false;
   }

	sgIP_dbgprint("Bind oK\n");

   return true;
}

bool CUnBlockingSocket::Listen() const
{
   ASSERT( m_hSocket!=SOCKET_UNDEF );
   if( net_listen(m_hSocket, 10)<0 )
   {
	sgIP_dbgprint("Listen Ko\n");

	   return false;
   }
   sgIP_dbgprint("Listen Ok\n");

   return true;
}


bool CUnBlockingSocket::Accept() 
{
   ASSERT( m_hSocket!=SOCKET_UNDEF );
    m_aSocket = m_hSocket;
	struct sockaddr_in data_peer_address;
	time_t end = time(NULL);
	time_t start = time(NULL);
    time_t time_now = 0;
	do
    {
		memset(&data_peer_address, 0, sizeof(struct sockaddr));
        u32 addrlen = sizeof(data_peer_address);
		m_hSocket = net_accept( m_aSocket, (struct sockaddr *)&data_peer_address, &addrlen);
        time_now = time(NULL); 
    } while((m_hSocket == -EAGAIN && time_now - start < 5) || time_now - end > 10); // try 5 second - 10 max

	if( m_hSocket<0 ) 
	{ 
	  m_hSocket = m_aSocket;
	  m_aSocket = SOCKET_UNDEF;
      return false;
    }
	  
   IPAddr = data_peer_address.sin_addr.s_addr; 

   m_connected = true;
 
  return true;
}



bool CUnBlockingSocket::Connect(LPCSOCKADDR psa) 
{
	u64 t1,t2;
    s32 ret;

	sgIP_dbgprint("trying connection");
   ASSERT( m_hSocket!=SOCKET_UNDEF );
   // should timeout by itself
 	set_blocking(m_hSocket, false );

   t1=ticks_to_millisecs(gettime());
    do
    {
        ret = net_connect( m_hSocket, (struct sockaddr*)psa, sizeof(struct sockaddr) );
        t2=ticks_to_millisecs(gettime());
        usleep(1000);
        if(t2-t1 > NET_TIMEOUT) break; // 10 secs to try to connect to handle->server_addr
    } while(ret!=-EAGAIN);

    if( ret!=-EISCONN )
    {
		sgIP_dbgprint("error connect %d",ret);
         return false;
    }
        
   if (eSockType==ApplicationSocket) {
     struct sockaddr_in * p = ( struct sockaddr_in *)psa;
	  
	  IPAddr = ntohl(p->sin_addr.s_addr); 
	   m_connected=true;
   }
	sgIP_dbgprint("connected");

//	set_blocking(m_hSocket, true); 

   return true;
}

u32 CUnBlockingSocket::Write(const char* pch, u32 nSize, s32 nSecs) const
{
   u32         nBytesSent        = 0;
   u32         nBytesThisTime;
   const char* pch1              = pch;

   do
   {
      nBytesThisTime = Send(pch1, nSize - nBytesSent, nSecs);
	  if (nBytesThisTime==0) return 0;
      nBytesSent += nBytesThisTime;
      pch1 += nBytesThisTime;
   } while( nBytesSent<nSize );

   return nBytesSent;
}



u32 CUnBlockingSocket::Send(const char* pch, u32 nSize, s32 nSecs) const
{
   ASSERT( m_hSocket!=SOCKET_UNDEF );
    u64 t1,t2;
	u32 wl = 0;
	s32 k = 0;
    t1=ticks_to_millisecs(gettime());
	while(wl != nSize ) {
	   k = net_write(m_hSocket , pch + wl, nSize - wl);
       if(k==-EAGAIN)
            {
             t2=ticks_to_millisecs(gettime());
            if( (t2 - t1) > NET_TIMEOUT)
				{
					sgIP_dbgprint("socket send timeout");
                    return 0;   /* timeout */
				}
            usleep(100);    /* allow system to perform work. Stabilizes system */
            continue; 
			}
		if(k < 0)
			 return 0;
		wl += k;
		t1=ticks_to_millisecs(gettime());
	}
	return wl;
}


s32 CUnBlockingSocket::ReceiveLine(char* buf, u32 len, s32 nSecs) const
{
        u32 i = 0, out = 0;
        s32 l;
		buf[0]=0;
		
		do{
                if( (l = ReceiveCmd(&(buf[i]), 1, nSecs)) > 0 )
                {
                        if(buf[i] == '\n') out = 1;
                        i++;
                }
                else
                {
                    buf[i] = 0;
					if (l<0) return -1;
                    return 0;
                }
        }while((i < (len-1))&&(out == 0));

        buf[i] = 0;

        return i;
}

s32 CUnBlockingSocket::ReceiveCmd(char* buf, u32 count, s32 nSecs) const
{
   ASSERT( m_hSocket!=SOCKET_UNDEF );
    s32 ret;
    u32 received = 0;
	s32 retsave=-131;
	u32 i=0;

	while(1)
	{
		time_t end = time(NULL);
		time_t start = time(NULL);
		time_t time_now = 0;
		do
		{
			ret=net_read(m_hSocket,buf,count);
			if (ret !=retsave&&count!=1){sgIP_dbgprint("r.%d",ret); retsave=ret;}
			if (ret!= -EAGAIN) break;
			time_now = time(NULL); 
		} while((ret == -EAGAIN && time_now - start < 5) || (ret<0 && time_now - end < 10)); // try 5 second - 10 max

		if (ret == -EAGAIN)
		{
			return 0;
		}
		else
		{
			i=0;
			buf+=ret;
			count-=ret;
			received+=ret;
			if(count==0 ||  ret == 0)  
			{
			return received;
			}
		}
	}
    return received;
}

s32 CUnBlockingSocket::Receive(char* buf, u32 count, s32 nSecs) const
{
	ASSERT( m_hSocket!=SOCKET_UNDEF );
    s32 ret;
	s32 retsave=-131;

	time_t end = time(NULL);
	time_t start = time(NULL);
    time_t time_now = 0;
	do
    {
		ret=net_read(m_hSocket,buf,count);
		if (ret!= -EAGAIN) break;
		if (ret<0 &&  time_now - end > 10) break;
		if (ret !=retsave&&count!=1){sgIP_dbgprint("r.%d",ret); retsave=ret;}
        time_now = time(NULL); 
    } while((ret == -EAGAIN && time_now - start < 5)); // try 5 second - 10 max

	if (ret == -EAGAIN)
	{
		return 0;
	}

    return ret;
}

s32 CUnBlockingSocket::ReceiveDatagram(char* pch, s32 nSize, LPSOCKADDR psa, s32 nSecs) const
{
   ASSERT( m_hSocket!=SOCKET_UNDEF );
   // input buffer should be big enough for the entire datagram
   u32 nFromSize = sizeof(SOCKADDR);
   s32 nBytesReceived = net_recvfrom(m_hSocket, pch, nSize, 0, psa, &nFromSize);

   if( nBytesReceived==SOCKET_ERROR )
   {
	   return 0;
   }

   return nBytesReceived;
}

s32 CUnBlockingSocket::SendDatagram(const char* pch, s32 nSize, LPCSOCKADDR psa, s32 nSecs) const
{
	ASSERT( CUnBlockingSocket::m_hSocket!=SOCKET_UNDEF );
   s32 nBytesSent = net_write(m_hSocket, pch, nSize);
   if( nBytesSent<0 )
   {
      return 0;
   }

   return nBytesSent;
}


bool CUnBlockingSocket::GetSockAddr(LPSOCKADDR psa) const
{
   ASSERT( m_hSocket!=SOCKET_UNDEF );
 
 // gets the address of the socket at this end
    CSockAddr csaLocalAddress(m_hSocket, (u32 )net_gethostip());
   psa = csaLocalAddress;

   return true;
}

extern char *
my_inet_ntoa(struct in_addr ina);
u32 IP4 (u8 a,u8 b,u8 c, u8 d) {
return (htonl(((u32)(a&0xff)<<24)|((u32)(b&0xff)<<16)|((u32)(c&0xff)<<8)|(u32)(d&0xff)));
}

CSockAddr CUnBlockingSocket::GetHostByName(const char* pchName, u32 ushPort /* = 0 */)
{
	u32 hostIp; 
	
   // hostent* pHostEnt = gethostbyname(pchName);
   struct hostent* hostEntry = net_gethostbyname(pchName);
        
  if ( hostEntry != NULL && hostEntry->h_length>0 && 
	   hostEntry->h_addr_list != NULL && hostEntry->h_addr_list[0]!=NULL )
        {
            struct in_addr addr;
            addr.s_addr = *(u32*)(hostEntry->h_addr_list[0]);
            hostIp =  addr.s_addr;
			sgIP_dbgprint("Resolve address: %s -> %s\n", pchName, inet_ntoa(addr));
        }
        else
        {
            hostIp = inet_addr( pchName );
			sgIP_dbgprint("Resolve address: %s -> %s\n", pchName, pchName ); 
        }

   // unsigned long* pulAddr = (unsigned long*) pHostEnt->h_addr_list[0];
   SOCKADDR_IN sockTemp;
   sockTemp.sin_family = AF_INET;
   sockTemp.sin_port = htons(ushPort);
   sockTemp.sin_addr.s_addr = hostIp; // IP4 (192, 168, 1, 2); // ConvertIPAddress(pchName);

	sgIP_dbgprint("trying with port %d %s %s",sockTemp.sin_port,pchName,my_inet_ntoa(sockTemp.sin_addr));

   //sockTemp.sin_addr.s_addr = ConvertIPAddress();
   // sockTemp.sin_addr.s_addr = *pulAddr; // address is already in network byte order

   return sockTemp;
}

