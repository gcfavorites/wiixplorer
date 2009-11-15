/*
 * txt.cpp
 * Class to get a langfile download link
 * 
 */
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#include "txt.h"


TXT::TXT(void) {
    iCntLine = 0;
}

TXT::~TXT(void) {
}

int TXT::getCnt() {
    return iCntLine;
}

const char * TXT::getLang(int nr) {
    return sLine[nr].c_str();
}

const char * TXT::getRev(int nr) {
    return sRev[nr].c_str();
}

int TXT::openLangTxtfile(const char * filename) {
    ifstream filestr;
    int i = 0;
    string str;
    filestr.open(filename);
	
	if (filestr.fail())
        return 0;

    filestr.seekg(0,ios_base::end);
    int size = filestr.tellg();
    if (size <= 0) return 0;
    filestr.seekg(0,ios_base::beg);
	
	while (!filestr.eof()) {
		
        getline(filestr,sLine[i]);
		i++;
		if (i == MAXLANG)
			break;
    }
    iCntLine = i;
	filestr.close();
    return 1;
}

int TXT::openRevTxtfile(const char * filename) {
	ifstream filestr;
    int i = 0;
    string str;
    filestr.open(filename);
	
	if (filestr.fail())
        return 0;

    filestr.seekg(0,ios_base::end);
    int size = filestr.tellg();
    if (size <= 0) return 0;
    filestr.seekg(0,ios_base::beg);
	
	while (!filestr.eof()) {
		getline(filestr,sRev[i]);
		i++;
		if (i == MAXREV)
			break;
    }
	filestr.close();
    return 1;
}

