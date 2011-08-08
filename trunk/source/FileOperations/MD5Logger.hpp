/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
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
 * for WiiXplorer 2010
 ***************************************************************************/
#ifndef MD5LOGGER_HPP_
#define MD5LOGGER_HPP_

#include <vector>
#include "ItemMarker.h"

class MD5Logger
{
	public:
		MD5Logger();
		~MD5Logger();
		bool LogMD5(const char * logpath, ItemMarker * Marker, bool showprogress = true);
	private:
		bool OpenLog(const char * path);
		void CloseLog();
		bool CalculateFile(const char * filepath, bool showprogress);
		bool CalculateDirectory(const char * path, bool showprogress);
		void SortList(std::vector<char *> & List);
		static bool SortCallback(const char * path1, const char * path2);
		void ClearList(std::vector<char *> & List);
		void CalcTotalSize(ItemMarker * Process);

		FILE * LogFile;
		int FolderCounter;
		int FileCounter;
		int ErrorCounter;
};

#endif
