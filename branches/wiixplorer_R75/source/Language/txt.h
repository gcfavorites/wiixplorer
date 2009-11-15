/*
 * txt.h
 * Class to get a langfile download link
 * 
 */

#ifndef _TXT_H
#define _TXT_H

#include <sstream>

#define MAXLANG			30
#define MAXREV			3


using namespace std;

class TXT {
private:
    string sLine[MAXLANG];
	string sRev[MAXREV];
    int iCntLine;

public:
    //!Constructor
    TXT(void);
    //!Destructor
    ~TXT(void);
    //!Open txt file with langs
    //!\param filename name of TXT file
    int openLangTxtfile(const char * filename);
	//!Open Rev file with RevNumber
    //!\param filename name of Rev file
    int openRevTxtfile(const char * filename);
	//!Gets Count langs
    //!\return Count langs
    int getCnt();
	//!Gets Name langs
    //!\return Name langs
	const char * getLang(int nr);
	//!Gets Rev Number
    //!\return Rev Number
	const char * getRev(int nr);
};

#endif
