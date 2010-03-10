/***************************************************************************
 * Copyright (C) 2010
 * by dude
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
 * Application Launcher Class
 *
 * for WiiXplorer 2010
 ***************************************************************************/
 
#ifndef _APPLICATIONS_H_
#define _APPLICATIONS_H_

#include <vector>

typedef struct
{
	char path[256];
	char name[256];
} Application;

class Applications
{
    public:
		static Applications *Instance();
        static void DestroyInstance();

		void Launch(int index);
		int Count() { return applications.size(); }
		char *GetName(int index) { return applications.at(index).name; }
		void Reload();

	private:
        Applications();
        ~Applications();

		static Applications *instance;

		std::vector<Application> applications;

		bool GetNameFromXML(char *xml, char *name);
		void Search();
};

#endif
