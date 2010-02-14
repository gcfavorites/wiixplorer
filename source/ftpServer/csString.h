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

#include <stdio.h>
#include <string.h>
//#include "tchar.h"
#include <stdarg.h>

#define _T(x)      x

#define _make_upper(str) _tcsupr(str)
#define _make_lower(str) _tcslwr(str)

// Maximum size of a text string in a resource file:
#define MAX_STRING_SIZE	1024

class CsString 
{
public:

	// Extended conversion of text into HTML:
	inline void ShiftLeft(int FirstPos, int nSymbols);
	void SmartTrim(bool bCutSpaces = false);
	void AdjustCodePage(long CodePage);
	void PostAllocate(int size = -1);
	int Count(char symbol);
	// Constructors:
	CsString ();
	CsString (const char * txt);
//	CsString (unsigned int  nID);
	CsString (const unsigned int  nID);
	CsString (CsString  & object);
	// Destructor:
	virtual ~CsString ();
		
	CsString  Left(int nCount);
	CsString  Right(int nCount);
	CsString  Mid(int nFirst, int nCount);

// simple formatting
	bool  Format(char * lpszFormat, ...);
	bool  Format(unsigned int nFormatID, ...);
	bool FormatV(char * lpszFormat, va_list argList);

	// Shrinks the actual buffer to the size of the string
	void Shrink();

	// Shrinks the actual buffer to the size of the string
	// if the actual buffer is at least twice as long as the
	// string length, and the text length is at least 16 symbols
	// (8 for Unicode)
	void ShrinkEffective();

	// This function is to allocate some memory for the pointer
	// in order to provide for faster memory operations.
	// It returns the buffer pointer.
	// Call PostAllocate after this method to set the correct string size.
	// Use sequential call to Shrink or ShrinkEffective after this
	// to avoid unnecessary memory being allocated.
	char * PreAllocate(int size);

	bool IsEmpty();
	void Empty();
	int GetLength();

	// Compares two strings. Returns:
	// < 0, if this string is less than parameter;
	// < 0, if this string is bigger than the parameter;
	//   0 - if the strings are equal.
	int Compare(const char * lpsz);
	int CompareNoCase(const char * lpsz);

	int FindOneOf(const char * lpszCharSet);
	int Find(char ch);
	int Find(char ch, int nStart);
	int Find(const char * lpszSub);
	int Find(const char * lpszSub, int nStart);
	int ReverseFind(char ch);

	char GetAt(int nIndex);
	void SetAt(int nIndex, char ch);

	char operator[](int nIndex);
	CsString  & operator = (const char * txt);
	CsString  & operator = (CsString  & object);
	CsString  & operator += (const char * txt);
	CsString  & operator += (char symbol);
	bool operator == (const char * txt);
	bool operator != (const char * txt);
    CsString  operator + ( CsString );
	CsString  operator + (const char * txt);
	operator const char *();

	CsString  SpanIncluding(const char * lpszCharSet);
	CsString  SpanExcluding(const char * lpszCharSet);

	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	int Replace(char chOld, char chNew);
	int Remove(char chRemove);
	int Delete(int nIndex, int nCount = 1);

	void TrimLeft(const char * lpszTargets);
	void TrimLeft(char chTarget);
	void TrimLeft();
	void TrimRight(const char * lpszTargetList);
	void TrimRight(char chTarget);
	void TrimRight();

	int TakeFromANSI(const char * buffer, int Length, long CodePage);

	
/*
CsString a1 ("/hello/lavie/"); 
i = a1.ReverseFindOccurence("/",0);   // -1 
i = a1.ReverseFindOccurence("/hello",1);   // 0 
i = a1.ReverseFindOccurence("/",1);   // 12 
i = a1.ReverseFindOccurence("/",2);   // 6 
i = a1.ReverseFindOccurence("/",3);   // 0 
i = a1.ReverseFindOccurence("/",4);   // -1 
i = a1.ReverseFindOccurence("bol",1); // -1 
i = a1.ReverseFindOccurence("la",1);   // 7 
i = a1.ReverseFindOccurence("vie",1);  // 9 
i = a1.ReverseFindOccurence("e",1);    // 11 

CsString k1 ("k"); 
i = k1.ReverseFindOccurence("jjjjj",1);   // -1 

CsString l1 ("k"); 
i = l1.ReverseFindOccurence("k",1);   // 0
i = l1.ReverseFindOccurence("k",2);   // -1

CsString b; 
CsString c1 ("kado/hello/lavie/se dechaine"); 
bool cr; 
cr = c1.ReverseFindOccurenceInBetween("/",0,&b);           // false, empty 
cr = c1.ReverseFindOccurenceInBetween("/hello/",1,&b);     // true, kado
cr = c1.ReverseFindOccurenceInBetween("/",1,&b);           // true, "se dechaine" 
cr = c1.ReverseFindOccurenceInBetween("/",2,&b);           // true, "lavie" 
cr = c1.ReverseFindOccurenceInBetween("/",3,&b);           // true, "hello" 
cr = c1.ReverseFindOccurenceInBetween("/",4,&b);           // true, kado 
cr = c1.ReverseFindOccurenceInBetween("/",5,&b);           // false, empty 
cr = c1.ReverseFindOccurenceInBetween("bol",4,&b);         // false, empty 
cr = c1.ReverseFindOccurenceInBetween("lavie",1,&b);       // true, "/se dechaine" 
cr = c1.ReverseFindOccurenceInBetween("lavie",2,&b);       // true "kado/hello/"
cr = c1.ReverseFindOccurenceInBetween("l",1,&b);           // true, "avie/se dechaine/” 
cr = c1.ReverseFindOccurenceInBetween("l",2,&b);           // true, "o/" 
cr = c1.ReverseFindOccurenceInBetween("l",3,&b);           // true, empty
cr = c1.ReverseFindOccurenceInBetween("l",4,&b);           // true, "kado/he" 
cr = c1.ReverseFindOccurenceInBetween("l",5,&b);           // false, empty 

CsString e1 ("//"); 
cr = e1.ReverseFindOccurenceInBetween("/",2,&b);           // true, empty 

CsString f1 ("/ba/b"); 
cr = f1.ReverseFindOccurenceInBetween("/b",2,&b);           // true, "a" 

CsString g1 ("/cadeau/cadeau"); 
cr = g1.ReverseFindOccurenceInBetween("/cadeau",2,&b);      // true, empty 


CsString h2 ("yyy/hello/lavie/"); 

cr = h2.LeftReverseFindOccurence("/",0,&b);     // false 
cr = h2.LeftReverseFindOccurence("/",1,&b);     // true, "yyy/hello/lavie" 
cr = h2.LeftReverseFindOccurence("/",2,&b);     // true, "yyy/hello" 
cr = h2.LeftReverseFindOccurence("/",3,&b);     // true, "yyy" 
cr = h2.LeftReverseFindOccurence("/",4,&b);     // false, empty 
cr = h2.LeftReverseFindOccurence("/",5,&b);     // false, empty 
cr = h2.LeftReverseFindOccurence("bol",4,&b);   // false, empty 
cr = h2.LeftReverseFindOccurence("lavie",1,&b); // true, "yyy/hello/" 
cr = h2.LeftReverseFindOccurence("lavie",2,&b); // false empty 
cr = h2.LeftReverseFindOccurence("l",1,&b);     // true, "yyy/hello/" 
cr = h2.LeftReverseFindOccurence("l",2,&b);     // true, "yyy/hel" 
cr = h2.LeftReverseFindOccurence("l",3,&b);     // true, "yyy/he" 
cr = h2.LeftReverseFindOccurence("l",4,&b);     // false, empty 
cr = h2.LeftReverseFindOccurence("l",5,&b);     // false, empty 

CsString a2 ("calendriergateaugateau"); 

cr = a2.LeftReverseFindOccurence("gateau",1,&b); // true, “calendriergateau" 
cr = a2.LeftReverseFindOccurence("gateau",2,&b); // true, “calendrier” 
cr = a2.LeftReverseFindOccurence("gateau",3,&b); // false 

CsString a ("/hello/laviy/"); 
a.FindOccurence("/",0);   // -1 
a.FindOccurence("/",1);   // 0 
a.FindOccurence("/",2);   // 6
a.FindOccurence("/",3);   // -1 
a.FindOccurence("bol",1); // -1 
a.FindOccurence("la",1);    // 8 
a.FindOccurence("viy",1);   // 9 
a.FindOccurence("y",1);     // 11

CsString g ("kkkk/hello/laviy/jdjjdj"); 
cr = g.FindOccurenceInBetween("/",0,&b);     // false, empty 
cr = g.FindOccurenceInBetween("/",1,&b);     // true, "kkkk"
cr = g.FindOccurenceInBetween("/",2,&b);     // true, "hello" 
cr = g.FindOccurenceInBetween("/",3,&b);     // true, "laviy" 
cr = g.FindOccurenceInBetween("/",4,&b);     // true, "jdjjdj" 
cr = g.FindOccurenceInBetween("/",5,&b);     // false, empty 
cr = g.FindOccurenceInBetween("bol",4,&b);   // false, empty 
cr = g.FindOccurenceInBetween("laviy",1,&b); // true, "kkkk/hello/" 
cr = g.FindOccurenceInBetween("laviy",2,&b); // true, "/jdjjdj"" 
cr = g.FindOccurenceInBetween("l",1,&b);     // true, "kkkk/he" 
cr = g.FindOccurenceInBetween("l",2,&b);     // true, empty 
cr = g.FindOccurenceInBetween("l",3,&b);     // true, "o/jdjjdj" 
cr = g.FindOccurenceInBetween("l",4,&b);     // true, "avie/" 
cr = g.FindOccurenceInBetween("l",5,&b);     // false, empty 

CsString c ("/hello/laviy/"); 

cr = c.LeftFindOccurence("laviy",1,&b); // true, "/hello/" 
cr = c.LeftFindOccurence("laviy",2,&b); // false, empty
cr = c.LeftFindOccurence("/",1,&b);     // true, empty
cr = c.LeftFindOccurence("/",2,&b);     // true, "/hello"
cr = c.LeftFindOccurence("/",3,&b);     // true, "/hello/laviy" 
cr = c.LeftFindOccurence("/",4,&b);     // false, empty 
cr = c.LeftFindOccurence("/",5,&b);     // false, empty 
*/
	int FindOccurence(const char * lpszSub, int occurs) ;
	bool FindOccurenceInBetween(const char * lpszSub, int occurs, CsString *result) ;
	bool LeftFindOccurence(const char * lpszSub, int occurs, CsString *result);
	int ReverseFind(const char *string, int nStart);
	int ReverseFindOccurence(const char * lpszSub, int occurs) ;
	int ReverseFindOccurence(const char ch, int occurs) ;
	bool ReverseFindOccurenceInBetween(const char * lpszSub, int occurs, CsString *result) ;
	bool LeftReverseFindOccurence(const char * lpszSub, int occurs, CsString *result) ;


// My own functions ****************************************************
	
	// Retrieve a text string which doesn't have any of the
	// exclusion symbols, and set 'nStart' and 'nLength'
	// accordingly.
	// The search begins with index 'nStart'
	CsString  GetExcluding(const char * exclusion, int & nStart, int & nLength);

	static bool _IsValidString(char * lpsz, int nLength = -1)
	{
		if(lpsz == NULL)
			return false;
		return true;
	}


private:

	const char * Strcpy(const char * text);
	const char * Strncpy(const char * text, int nSymbols);
	void Release();
	
	// Adds txt to the existing text
	void AddText(const char * txt, int Length);

private:

	char * m_pText;
	int m_uSize;
	int m_uBufferSize;
};
