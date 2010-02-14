
/****************************************************************************
 * Copyright (C) 2009
 * by LilouMaster based D. J. Bernstein (C ==> C++)
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <zlib.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <ctype.h>
#include <string>
#include "FTPClientDefinements.h"
#include "FTPClientconfig.h"

#ifndef INC_FTPLISTPARSE_H
#define INC_FTPLISTPARSE_H



namespace nsFTP
{

class CFTPFileStatus;

/// Implements the parsing of the string returned by the LIST command.
class CFTPListParse
{
public:
   CFTPListParse();
   ~CFTPListParse();

public:
   bool Parse(CFTPFileStatus& ftpFileStatus, const tstring& strLineToParse);

   bool IsEPLS(LPCTSTR pszLine);
   bool ParseEPLF(CFTPFileStatus& ftpFileStatus, LPCTSTR pszLine, int iLength);

   bool IsUNIXStyleListing(LPCTSTR pszLine);
   bool ParseUNIXStyleListing(CFTPFileStatus& ftpFileStatus, LPCTSTR pszLine, int iLength);

   bool IsMultiNetListing(LPCTSTR pszLine);
   bool ParseMultiNetListing(CFTPFileStatus& ftpFileStatus, LPCTSTR pszLine, int iLength);

   bool IsMSDOSListing(LPCTSTR pszLine);
   bool ParseMSDOSListing(CFTPFileStatus& ftpFileStatus, LPCTSTR pszLine, int iLength);

private:
   bool CheckMonth(LPCTSTR pszBuffer, LPCTSTR pszMonthName) const;
   int  GetMonth(LPCTSTR pszBuffer, int iLength) const;
   bool GetLong(LPCTSTR pszLong, int iLength, long& lResult) const;
   long GetYear(time_t time) const;

   long ToTAI(long lYear, long lMonth, long lMDay) const;
   long GuessTAI(long lMonth, long lMDay);

private:
   time_t        m_tmBase;       // time() value on this OS at the beginning of 1970 TAI
   long          m_lCurrentYear; // approximation to current year
   static TCHAR* m_Months[12];
};

}

#endif // INC_FTPLISTPARSE_H
