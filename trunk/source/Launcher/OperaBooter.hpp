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
#ifndef BOOTOPERA_HPP_
#define BOOTOPERA_HPP_

#include <vector>
#include <gccore.h>
#include <mxml.h>

typedef struct
{
    char * name;
    char * addr;
    bool downloadlink;
} Link;

class OperaBooter
{
    public:
        OperaBooter(const char * xmlfile);
        ~OperaBooter();
        bool Launch(int pos);
        bool SaveXML();
        bool AddLink();
        bool AddLink(const char * name, const char * addr, bool downloadlink);
        void RemoveLink(int pos);
        const char * GetName(int pos);
        const char * GetLink(int pos);
        int GetCount() { return LinkList.size(); };
    private:
        bool ParseXML();
        void ClearList();
        bool DownloadFile(int pos);
        void Sort();
        static bool SortCallback(const Link & f1, const Link & f2);

        std::vector<Link> LinkList;
        std::string FilePath;
        u8 * xmlbuffer;
        mxml_node_t * xmlfile;
        u64 OperaID;
};

#endif
