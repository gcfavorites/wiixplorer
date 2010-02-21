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
 
// #include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "csString.h"
#include <stdarg.h>


//#include "char.h"

#define TUCHAR const unsigned char
#define TCHAR   char
#define WORD int 
#define ATLASSERT(exp) if ( !(exp) ) return false
inline  char *CharNext( char* pcPtr) { return(	 pcPtr + 1 ); }
inline  char *CharPrev( char * /* pcStart */,  char* pcPtr) { return (pcPtr - 1 ); }
#ifndef min
#  define min(x,y)  ((x)>(y)?(y):(x))
#endif

#ifndef max
#  define max(x,y)  ((x)<(y)?(y):(x))
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CsString ::CsString (CsString  & object)
{
	m_uSize = 0;
	m_uBufferSize = 0;
	m_pText = NULL;
	Strncpy(object.m_pText, object.m_uSize);
}

CsString ::CsString ()
{
	m_uSize = 0;
	m_uBufferSize = 0;
	m_pText = NULL;
}

CsString ::CsString (const char * txt)
{
	m_uSize = 0;
	m_uBufferSize = 0;
	m_pText = NULL;
	Strcpy(txt);
}


CsString ::~CsString ()
{
	Release();
}

CsString ::CsString (const unsigned int size)
{
	m_uSize = 0;
	m_uBufferSize = size;
	m_pText = new char[size + 1];
}
CsString  CsString ::  operator + ( CsString c )
{
    CsString sum ;
	sum = *this;
	sum += c;
    return sum;
}


CsString  CsString ::   operator + (const char * txt)
{
	CsString sum ;
	sum = *this;
	sum += txt;
    return sum;
}

bool CsString ::IsEmpty()
{
	return !m_uSize;
}

const char * CsString ::Strcpy(const char * text)
{
	return Strncpy(text, text?strlen(text):(unsigned int)NULL);
}

const char * CsString ::Strncpy(const char * text, int nSymbols)
{
	if(!text)
	{
		Release();
		return NULL;
	}
	if(nSymbols > m_uBufferSize)
	{
		Release();
		m_pText = new char[nSymbols + 1];
		m_uBufferSize = nSymbols;
	}
	if(!m_pText)
		return NULL;
	
	strncpy(m_pText, text, nSymbols);
	m_pText[nSymbols] = 0;
	m_uSize = nSymbols;
	return m_pText;
}


void CsString ::Release()
{
	m_uSize = 0;
	m_uBufferSize = 0;
	if(m_pText)
	{
		delete []m_pText;
		m_pText = NULL;
	}
}

int CsString ::GetLength()
{
	return m_uSize;
}

int CsString ::Compare(const char * lpsz)
{
	if(!m_uSize)
		return -1;
	if(!lpsz)
		return 1;
	return strcmp(m_pText, lpsz);
}

int CsString ::CompareNoCase(const char * lpsz)
{
	if(!m_uSize)
		return -1;
	if(!lpsz)
		return 1;
	return stricmp(m_pText, lpsz);
}

int CsString ::FindOneOf(const char * lpszCharSet)
{
	if(!lpszCharSet || !m_uSize)
		return -1;
	char * lpsz = strpbrk(m_pText, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pText);
}

int CsString ::Find(char ch)
{
	return Find(ch, 0);
}

int CsString ::Find(char ch, int nStart)
{
	if(m_uSize <= nStart)
		return -1;
	char * lpsz = strchr(m_pText + nStart, (unsigned char)ch);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pText);
}

int CsString ::Find(const char * lpszSub)
{
	return Find(lpszSub, 0);
}

int CsString ::Find(const char * lpszSub, int nStart)
{
	if(m_uSize <= nStart || !lpszSub)
		return -1;
	char * lpsz = strstr(m_pText + nStart, lpszSub);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pText);
}

int CsString ::ReverseFind(char ch)
{
	if(!m_uSize)
		return -1;
	char * lpsz = strrchr(m_pText, (unsigned char)ch);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pText);
}

void CsString ::Empty()
{
	Release();
}

char CsString ::GetAt(int nIndex)
{
#ifdef _DEBUG
	if(nIndex > 0 && nIndex < m_uSize)
		return m_pText[nIndex];
	return -1;
#else
	return m_pText[nIndex];
#endif
}

void CsString ::SetAt(int nIndex, char ch)
{
#ifdef _DEBUG
	if(nIndex > 0 && nIndex < m_uSize)
#endif
		m_pText[nIndex] = ch;
}

CsString  & CsString ::operator = (const char * txt)
{
	Strcpy(txt);
	return *this;
}


CsString  & CsString ::operator = (CsString  & object)
{
	Strncpy(object.m_pText, object.m_uSize);
	return *this;
}

CsString  & CsString ::operator += (const char * txt)
{
	if(!txt)
		return *this;

	AddText(txt, strlen(txt));
	return *this;
}

CsString  & CsString ::operator += (char symbol)
{
	AddText(&symbol, 1);
	return *this;
}

char CsString ::operator[](int nIndex)
{
#ifdef _DEBUG
	if(nIndex >= 0 && nIndex < m_uSize)
		return m_pText[nIndex];
	return -1;
#else
	return m_pText[nIndex];
#endif
}

bool CsString ::operator == (const char * txt)
{
	return !Compare(txt);
}

bool CsString ::operator != (const char * txt)
{
	return Compare(txt)?true:false;
}

CsString ::operator const char *()
{
	return m_pText;
}

void CsString ::AddText(const char * txt, int Length)
{
	if(txt)
	{
		if(!Length)
			return;

		if(Length + m_uSize > m_uBufferSize)
		{
			char * tmp = new char[sizeof(char)*(Length + m_uSize + 1)];
			strncpy(tmp, m_pText, m_uSize);
			strncpy(tmp + m_uSize, txt, Length);
			tmp[m_uSize + Length] = 0;

			if(m_pText)
				delete []m_pText;
			m_pText = tmp;
			m_uSize += Length;
			m_uBufferSize = m_uSize;
		}
		else
		{
			strncpy(m_pText + m_uSize, txt, Length);
			m_uSize += Length;
			m_pText[m_uSize] = 0;
		}
	}
}

void CsString ::Shrink()
{
	if(m_uBufferSize > m_uSize)
	{
		if(m_uSize)
		{
			m_pText = (char *)realloc(m_pText, (m_uSize + 1) * sizeof(char));
			m_pText[m_uSize] = 0;
			m_uBufferSize = m_uSize;
		}
		else
			Release();
	}
}

void CsString ::ShrinkEffective()
{
	if (m_uSize * sizeof(char) > 16 && m_uBufferSize >= m_uSize * 2)
	{
		m_pText = (char *)realloc(m_pText, (m_uSize + 1) * sizeof(char));
		m_pText[m_uSize] = 0;
		m_uBufferSize = m_uSize;
	}
}

CsString  CsString ::Left(int nCount)
{
	if (nCount < 0)
		nCount = 0;
	if(nCount >= m_uSize)
		return *this;
	
	CsString  dest;
	dest.Strncpy(m_pText, nCount);
	return dest;
}

CsString  CsString ::Right(int nCount)
{
	if (nCount < 0)
		nCount = 0;
	if(nCount >= m_uSize)
		return *this;
	
	CsString  dest;
	dest.Strncpy(m_pText + m_uSize - nCount, nCount);
	return dest;
}

CsString  CsString ::Mid(int nFirst, int nCount)
{
	if(!nFirst)
		return Left(nCount);

	CsString  dest;
	if (nCount < 0)
		nCount = 0;
	if(nFirst >= m_uSize)
		return dest;
	if(nFirst + nCount > m_uSize)
		nCount = m_uSize - nFirst;
	
	dest.Strncpy(m_pText + nFirst, nCount);
	return dest;
}

CsString  CsString ::SpanIncluding(const char * lpszCharSet)
{
	if(!m_uSize)
		return CsString ();

	return Left(strspn(m_pText, lpszCharSet));
}

CsString  CsString ::SpanExcluding(const char * lpszCharSet)
{
	if(!m_uSize)
		return CsString ();

	return Left(strcspn(m_pText, lpszCharSet));
}


void CsString ::MakeUpper()
{
	if(!m_uSize)
		return;

	strupr(m_pText);
}


void CsString ::MakeLower()
{
	if(!m_uSize)
		return;

	strlwr(m_pText);
}

template<class T>
inline void
my_swap (T &t1, T &t2, T &tmp)
{
  tmp = t1;
  t1 = t2;
  t2 = tmp;
}

char *
strrev (char * byte, int length_i)
{
  char tmp_byte;

  char* left = byte;
  char* right = byte + length_i;

  while (right > left)
    {
      my_swap (*right-- , *left++ , tmp_byte);
    }
  return byte;
}
void CsString ::MakeReverse()
{
	if(!m_uSize)
		return;

	strrev(m_pText, m_uSize);
}

char *strinc(const char *str)
{
    return (char *)(str + 1);
}
int CsString ::Replace(char chOld, char chNew)
{
	int nCount = 0;
	if (chOld != chNew)
	{
		char * psz = m_pText;
		char * pszEnd = psz + m_uSize;
		while (psz < pszEnd)
		{
			if (*psz == chOld)
			{
				*psz = chNew;
				nCount++;
			}
			psz = strinc(psz);
		}
	}
	return nCount;
}

int CsString ::Remove(char chRemove)
{
	if(!m_uSize)
		return 0;

	char * pstrSource = m_pText;
	char * pstrDest = m_pText;
	char * pstrEnd = m_pText + m_uSize;
	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != chRemove)
		{
			*pstrDest = *pstrSource;
			pstrDest = strinc(pstrDest);
		}
		pstrSource = strinc(pstrSource);
	}
	if(m_pText)
		*pstrDest = '\0';
	int nCount = pstrSource - pstrDest;
	m_uSize -= nCount;
	return nCount;
}

int CsString ::Delete(int nIndex, int nCount /* = 1 */)
{
	if (nIndex < 0)
		nIndex = 0;
	int nNewLength = m_uSize;
	if (nCount > 0 && nIndex < nNewLength)
	{
		int nBytesToCopy = nNewLength - (nIndex + nCount) + 1;
		memcpy(m_pText + nIndex, m_pText + nIndex + nCount, nBytesToCopy * sizeof(char));
		m_uSize = nNewLength - nCount;
	}
	return nNewLength;
}

void CsString ::TrimLeft(const char * lpszTargets)
{
	if(!m_uSize)
		return;

	const char * lpsz = m_pText;
	while (*lpsz != '\0')
	{
		if (strchr(lpszTargets, *lpsz) == NULL)
			break;
		lpsz = strinc(lpsz);
	}
	if (lpsz != m_pText)
	{
		// fix up data and length
		int nDataLength = m_uSize - (lpsz - m_pText);
		memmove(m_pText, lpsz, (nDataLength + 1)*sizeof(char));
		m_uSize = nDataLength;
	}
}

void CsString ::TrimLeft(char chTarget)
{
	if(!m_uSize)
		return;
	// find first non-matching character
	const char * lpsz = m_pText;
	while (chTarget == *lpsz)
		lpsz = strinc(lpsz);
	if (lpsz != m_pText)
	{
		// fix up data and length
		int nDataLength = m_uSize - (lpsz - m_pText);
		memmove(m_pText, lpsz, (nDataLength+1)*sizeof(char));
		m_uSize = nDataLength;
	}
}

void CsString ::TrimLeft()
{
	if(!m_uSize)
		return;
	// find first non-space character
	const char * lpsz = m_pText;
	while (isspace(*lpsz))
		lpsz = strinc(lpsz);
	if (lpsz != m_pText)
	{
		// fix up data and length
		int nDataLength = m_uSize - (lpsz - m_pText);
		memmove(m_pText, lpsz, (nDataLength+1)*sizeof(char));
		m_uSize = nDataLength;
	}
}

void CsString ::TrimRight(const char * lpszTargetList)
{
	if(!m_uSize)
		return;

	char * lpsz = m_pText;
	char * lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (strchr(lpszTargetList, *lpsz) != NULL)
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = strinc(lpsz);
	}
	if (lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		m_uSize = lpszLast - m_pText;
	}
}

void CsString ::TrimRight(char chTarget)
{
	if(!m_uSize)
		return;

	char * lpsz = m_pText;
	char * lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (*lpsz == chTarget)
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = strinc(lpsz);
	}

	if (lpszLast != NULL)
	{
		// truncate at left-most matching character
		*lpszLast = '\0';
		m_uSize = lpszLast - m_pText;
	}
}

void CsString ::TrimRight()
{
	if(!m_uSize)
		return;

	char * lpsz = m_pText;
	char * lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (isspace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = strinc(lpsz);
	}

	if (lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = '\0';
		m_uSize = lpszLast - m_pText;
	}
}

CsString  CsString ::GetExcluding(const char * exclusion, int & nStart, int & nLength)
{
	CsString  result;
	if(!m_uSize || nStart > m_uSize || nStart < 0)
	{
		nLength = nStart = 0;
		return result;
	}
	CsString  excl(exclusion);
	int i = nStart;
	nLength = 0;
// Now we must find the beginning and the end of such text:
	while(i < m_uSize)
	{
		if(excl.Find(m_pText[i]) == -1)	// Normal text found
		{
			if(!nLength)
				nStart = i;
			nLength ++;						// Accumulating the length
		}
		else
			if(nLength)	// the normal text is over
				break;
		i ++;
	}
	if(nLength)
		result = Mid(nStart, nLength);
	return result;
}


char * CsString ::PreAllocate(int size)
{
	if(m_uBufferSize >= size)
		return m_pText;
	m_uBufferSize = size;
	if(m_pText)
		m_pText = (char *)realloc(m_pText, size * sizeof(char));
	else
		m_pText = new char[size + 1];
	return m_pText;
}

int CsString ::Count(char symbol)
{
	if(!m_uSize)
		return 0;
	int counter = 0;
	for(int i=0;i<m_uSize;i++)
		if(m_pText[i] == symbol)
			counter ++;
	return counter;
}

void CsString ::PostAllocate(int size)
{
	if(!m_uBufferSize)
		return;
	if(size != -1 && size < m_uBufferSize)
	{
		m_pText[size] = 0;
		m_uSize = size;
	}
	else
	{
		m_uSize = 0;
		int i = 0;
		while(i <= m_uBufferSize)
		{
			if(!m_pText[i])
			{
				m_uSize = i;
				return;
			}
			i ++;
		}
	}
}


// If the text doesn't have anything except spaces in it
// the text is emptied.
// It also removes all repetitious spaces, leaving single ones
// boundaries.
// The first flag is true when the first spaces are to be cut completely.
void CsString ::SmartTrim(bool bCutSpaces /* =false */)
{
	if(!m_uSize)
		return;

	bool bStart = true;
	int nSpaces = 0;
	bool bHasSymbols = false;
	for(int I = 0;I < m_uSize;I ++)
	{
		char symbol = m_pText[I];

		if(symbol == ' ' || symbol == '\n' || symbol == '\r' || symbol == '\t')
		{
			m_pText[I] = ' ';
			nSpaces ++;
		}
		else
		{
			bHasSymbols = true;
			if(bStart)
			{
				// Just reached the beginning of the phrase:
				bStart = false;
				int shift = 1;
				if(bCutSpaces)
					shift = 0;
				if(nSpaces > shift)
				{
					ShiftLeft(nSpaces, nSpaces - shift);
					I -= nSpaces - shift;
				}
				else
					I ++;
			}
			else
			{
				// Somewhere in between:
				if(nSpaces > 1)
				{
					ShiftLeft(I, nSpaces - 1);
					I -= nSpaces - 1;
					
				}
			}
			nSpaces = 0;
		}
	}
	if(!bHasSymbols)
		Empty();
	else
	{
		if(nSpaces > 1)
		{
			m_uSize -= nSpaces - 1;
			m_pText[m_uSize] = 0;
		}
		ShrinkEffective();
	}
}

// This function shifts contents of the string to the left
// by nSymbols symbols beginning from position FirstPos
// without reallocating memory.
void CsString ::ShiftLeft(int FirstPos, int nSymbols)
{
	strncpy(m_pText + FirstPos  - nSymbols, m_pText + FirstPos, m_uSize - FirstPos + 1);
	m_uSize -= nSymbols;
}


// formatting (using wsprintf style formatting)
bool CsString::Format(char * format, ...)
{


 
 
   char msg[200];
   msg[0]=0;
	ATLASSERT(_IsValidString(format));

  va_list ap;
  va_start(ap, format);
  if(vsprintf(msg, format, ap) > (200 - 1))    ;
  Strcpy(msg);

  va_end(ap);

	return true;
}




int CsString ::FindOccurence(const char * lpszSub, int occurs) 
{ 
	int pos=0; 
	int tpos=0; 
	int i=0; 

	if (occurs <= 0 ) 
		return -1; 

	for (i=0;i<occurs; i++) {   
		if ((tpos=Find(lpszSub, pos)) == -1) { 
			return -1; 
		} 
		else { 
			pos = tpos + strlen(lpszSub); 
		} 
	} 
	return tpos; 
} 


bool CsString ::FindOccurenceInBetween(const char * lpszSub, int occurs, CsString *result) 
{ 
 
	int tpos=0; 
	int bpos, epos; 
	CsString  dest; 
	const int len = strlen(lpszSub); 

	dest=""; 

	if (occurs <= 0 ) 
		goto FindOccurenceInBetween_Error; 



	if (occurs == 1 ) 
	{ // first occurence 
		tpos = FindOccurence(lpszSub, 1); 

		if (tpos == -1) 
		{ 
			goto FindOccurenceInBetween_Error; 
		} else 
		{ 
			if (tpos!=0) 
				dest.Strncpy(m_pText, tpos ); 
		} 
	} 
	else 
	{ 
		// more than one 
		bpos = this->FindOccurence(lpszSub, occurs-1); 
		epos = this->FindOccurence(lpszSub, occurs); 

		if (bpos == -1 || (bpos == epos && bpos == -1)) 
			goto FindOccurenceInBetween_Error; 

		if (epos != -1) 
		{ 
			bpos = bpos + len; 
			epos = epos - 1; 
		} 
		else 
		{ 
			bpos = bpos + len; 
			epos = m_uSize; 
		} 

		if (bpos<m_uSize)
			dest.Strncpy(m_pText+bpos, epos - bpos +1 ); 
	} 

	*result = dest; 
	return true; 

FindOccurenceInBetween_Error : 
	*result = dest; 

	return false; 
} 



bool CsString ::LeftFindOccurence(const char * lpszSub, int occurs, CsString *result) 
{ 

	int tpos=0; 
	int bpos, epos, clen; 
	CsString  dest; 


	dest=""; 

	if (occurs <= 0 ) 
		goto FindOccurenceInBetween_Error; 



	if (occurs == 1 ) 
	{ // first occurence 
		tpos = FindOccurence(lpszSub, 1); 

		if (tpos == -1) 
		{ 
			goto FindOccurenceInBetween_Error; 
		} else 
		{ 
			if (tpos!=0) 
				dest.Strncpy(m_pText, tpos ); 
		} 
	} 
	else 
	{ 
		// more than one 
		bpos = this->FindOccurence(lpszSub, occurs-1); 
		epos = this->FindOccurence(lpszSub, occurs); 

		if (bpos == -1 || epos == -1) 
			goto FindOccurenceInBetween_Error; 

		if (epos != -1) 
		{ 
			clen = epos ; 
		} 
		else 
		{ 
			clen = m_uSize; 
		} 

		if (bpos<m_uSize)
			dest.Strncpy(m_pText, clen ); 
	} 

	*result = dest; 
	return true; 

FindOccurenceInBetween_Error : 
	*result = dest; 

	return false; 
} 


char * 
strrstr(char *string, char *find, int len ) 
{ 
	size_t stringlen, findlen; 
	char *cp; 

	findlen = strlen(find); 
	stringlen = len+1 ; 
	if (findlen > stringlen) 
		return NULL; 

	for (cp = string + stringlen - findlen; cp >= string; cp--) 
		if (strncmp(cp, find, findlen) == 0) 
			return cp; 

	return NULL; 
} 



int CsString ::ReverseFind(const char *string, int nStart) 
{ 
	if(m_uSize <= nStart) 
		return -1; 
	char * lpsz = strrstr (m_pText  , (char *)string, nStart); 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pText); 
} 





int CsString ::ReverseFindOccurence(const char * lpszSub, int occurs) 
{ 
	int pos=m_uSize-1; 
	int tpos=0; 
	int i=0; 

	if (m_uSize==0 || occurs <= 0 ) 
		return -1; 

	for (i=0;i<occurs; i++) {   
		if ((tpos=ReverseFind(lpszSub, pos)) == -1) { 
			return -1; 
		} 
		else { 
			if (tpos == 0 && (!(occurs-1  == i))) { return -1; } 
			else {pos = tpos-1; }
		} 
	} 
	return tpos; 
} 

int CsString ::ReverseFindOccurence(const char ch, int occurs) 
{ 
	char lszSub[2]; 
	lszSub[0] = ch; 
	lszSub[1] = 0; 

	return (ReverseFindOccurence((const char * )lszSub, occurs)); 
} 



bool CsString::ReverseFindOccurenceInBetween(const char * lpszSub, int occurs, CsString *result) 
{ 

	int tpos=0; 
	int bpos, epos; 
	CsString dest; 
	const int len = strlen(lpszSub); 
	dest=""; 

	if (occurs <= 0 || len <=0) 
		goto ReverseFindOccurenceInBetween_Error; 


	if (occurs == 1) 
	{ // first occurence 
		tpos = ReverseFindOccurence(lpszSub, 1); 
		if (tpos == -1) 
		{ 
			goto ReverseFindOccurenceInBetween_Error; 
		} else 
		{ 
			if (tpos!=0) 
				dest.Strncpy(m_pText+tpos+len, m_uSize-tpos ); 
		} 
	} 
	else 
	{ 
		// more than one 
		epos = this->ReverseFindOccurence(lpszSub, occurs-1); 
		bpos = this->ReverseFindOccurence(lpszSub, occurs); 

		if (epos == -1 || (bpos == epos && bpos == -1)) 
			goto ReverseFindOccurenceInBetween_Error; 

		if (bpos!=-1) 
		{ 
			dest.Strncpy(m_pText+bpos+len, epos - (bpos + len) );  
		} 
		else 
		{ 
			dest.Strncpy(m_pText, epos );  
		} 
	} 
	*result = dest; 
	return true; 

ReverseFindOccurenceInBetween_Error : 
	*result = dest; 
	return false; 
} 






bool CsString ::LeftReverseFindOccurence(const char * lpszSub, int occurs, CsString *result) 
{ 

	int tpos=0; 
	int bpos, epos; 
	CsString dest; 
	const int len = strlen(lpszSub); 
	dest=""; 

	if (occurs <= 0 || len <=0) 
		goto LeftReverseFindOccurence_Error; 

	if (occurs == 1 ) 
	{ // first occurence 
		tpos = ReverseFindOccurence(lpszSub, 1); 
		if (tpos == -1) 
		{ 
			goto LeftReverseFindOccurence_Error; 
		} else 
		{ 
			if (tpos!=0) 
				dest.Strncpy(m_pText, tpos ); 
		} 
	} 
	else 
	{ 
		// more than one 
		epos = this->ReverseFindOccurence(lpszSub, occurs-1); 
		bpos = this->ReverseFindOccurence(lpszSub, occurs); 
		if (bpos == -1 || (bpos == epos && bpos == -1)) 
			goto LeftReverseFindOccurence_Error; 

		if (bpos != -1) 
		{ 
			dest.Strncpy(m_pText, bpos); 
		} 
		else 
		{ 
			dest.Strncpy(m_pText, epos); 
		} 
	}

		*result = dest; 
		return true; 

LeftReverseFindOccurence_Error: 
		*result = dest; 
		return false; 
}

