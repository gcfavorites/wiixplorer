
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


#ifndef INC_DEFINEMENTS_H
#define INC_DEFINEMENTS_H

#include <string>
#include <set>

#include <errno.h>
#include <network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <memory>
#include <queue>
#include <time.h>

   typedef char* LPTSTR;
   typedef const char* LPCTSTR;
   typedef u32 UINT;
   typedef u32* PUINT;
   typedef char TCHAR;
   typedef const char* LPCSTR;
   typedef u32 DWORD;


   typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr  *LPSOCKADDR;

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in  *LPSOCKADDR_IN;

typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger  *LPLINGER;

typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr  *LPIN_ADDR;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;
typedef struct fd_set  *LPFD_SET;

typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent  *LPHOSTENT;

typedef struct servent SERVENT;
typedef struct servent *PSERVENT;
typedef struct servent  *LPSERVENT;

typedef struct protoent PROTOENT;
typedef struct protoent *PPROTOENT;
typedef struct protoent  *LPPROTOENT;

typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval  *LPTIMEVAL;

extern char * inet_ntoa(struct in_addr ina);
typedef s32        SOCKET;
extern int errno; 
#define WSAGetLastError() errno



//#define WSAGetLastError() errno

#ifndef VERIFY
#define VERIFY
#endif

#ifndef ASSERT
#define ASSERT(a) ; // assert (__LINE__, __FILE__, a)
#endif

// definements for unicode support
#ifdef _UNICODE
   typedef std::wstring tstring;
   typedef std::wostream tostream;
   typedef std::wofstream tofstream;
   typedef std::wostringstream tostringstream;
   typedef std::wistringstream tistringstream;
   typedef std::wstringstream tstringstream;
   typedef std::wstreambuf tstreambuf;

   #define tcin  wcin
   #define tcout wcout
   #define tcerr wcerr
   #define tclog wclog
   #define tcstoul wcstoul
   #define tcschr wcschr
   #define tcsncmp wcsncmp
   #define tisdigit iswdigit
   #define tsprintf swprintf
   #define tcsncpy wcsncpy
   #define ttol wcstol
   #define _sntprintf _snwprintf

   #ifndef _T
   #define _T(x) L ## x
   #endif
#else
   typedef std::string tstring;
   typedef std::ostream tostream;
   typedef std::ofstream tofstream;
   typedef std::ostringstream tostringstream;
   typedef std::istringstream tistringstream;
   typedef std::stringstream tstringstream;
   typedef std::streambuf tstreambuf;

   #define tcin  cin
   #define tcout cout
   #define tcerr cerr
   #define tclog clog
   #define tcstoul strtoul
   #define tcschr strchr
   #define tcsncmp strncmp
   #define tisdigit isdigit
   #define tsprintf sprintf
   #define tcsncpy strncpy
   #define ttol strtol
   #define _sntprintf(a,b,c,d)  printf("%s",c);

   #define FALSE   0
#define TRUE    1

#define SOCKET_ERROR            (-1)

extern void debugwii2(const char *format, ...);


   // extern "C" void sgIP_dbgprint(char * txt, ...) ;
   #define sgIP_dbgprint sgIP_dbgprint
   #define dbgprintclient sgIP_dbgprint
      #define dbgprintclient_R sgIP_dbgprint 
	     #define dbgprintclient_E sgIP_dbgprint 
		    #define dbgprintclient_S sgIP_dbgprint 
			
extern void sgIP_dbgprint(const char * txt, ...) ;

			/*
extern "C" void dbgprintclient(char * txt, ...) ;
extern "C" void dbgprintclient_R(char * txt, ...) ;
extern "C" void dbgprintclient_E(char * txt, ...) ;
extern "C" void dbgprintclient_S(char * txt, ...) ;
*/
   extern void assert (int , char *, int) ;
#define WSAEINTR                         10004L

   extern void sleep(int);




   #ifndef _T
   #define _T
   #endif


#endif

namespace nsHelper
{
#ifdef WIN32
   /// Class with static functions to get information about an error.
   class CError
   {
   public:
      static DWORD GetLastError() { return ::GetLastError(); }
   
      static tstring GetErrorDescription(int iErrorCode = GetLastError())
      {
         LPVOID lpMsgBuf=NULL;
         FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, iErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                     reinterpret_cast<LPTSTR>(&lpMsgBuf), 0, NULL);
         tstring strErrorDescription;
         if( lpMsgBuf )
         {
            strErrorDescription = reinterpret_cast<LPTSTR>(lpMsgBuf);
            LocalFree(lpMsgBuf);
         }
         return strErrorDescription;
      }
   };
#else
   /// Class with static functions to get information about an error.
   class CError
   {
   public:
      static DWORD GetLastError() { return static_cast<DWORD>(-1); }
   
      static tstring GetErrorDescription(int iErrorCode = GetLastError())
      {
         return ("ToDo: GetErrorDescription not implemented yet!");
      }
   };
#endif

/// Class with static functions to do string conversions.
class CCnv
{
public:
   static long TStringToLong(const tstring& strIn)
   {
      TCHAR* pStopString = 0;
   #ifdef _UNICODE
      return wcstol(&*strIn.begin(), &pStopString, 10);
   #else
      return strtol(&*strIn.begin(), &pStopString, 10);
   #endif
   }


   static tstring ConvertToTString(const char* szIn)
   {
      tstring strOut;
      return ConvertToTString(szIn, strOut);
   }

   static tstring& ConvertToTString(const char* szIn, tstring& strOut)
   {
   #ifdef _UNICODE
      if( strlen(szIn) == 0 )
      {
         strOut.clear();
      }
      else
      {
         strOut.resize(strlen(szIn));
         mbstowcs(&*strOut.begin(), szIn, strOut.size());
      }
   #else
      strOut = szIn;
   #endif
      return strOut;
   }
};


/// @brief Calculates elapsed CPU time.
///
/// Is useful for calculating transferrates.

class CTimer
{
public:
   CTimer() : m_dfStart(clock()) {}

   /// Restarts the timer.
   void Restart() { m_dfStart = clock(); }

   /// Get the elapsed time in seconds.
   double GetElapsedTime() 
   {
      return (clock() - m_dfStart)/CLOCKS_PER_SEC;
   }

private:
   double m_dfStart; ///< elapsed CPU time for process (start of measurement)
   double clock(void) { time_t t; time(&t); return t;}
};

}

#endif // INC_DEFINEMENTS_H
