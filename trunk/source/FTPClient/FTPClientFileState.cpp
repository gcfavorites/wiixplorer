
/****************************************************************************
 * Copyright (C) 2009
 * by LilouMaster based Copyright (c) 2004 Thomas Oswald
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

#include "FTPClientconfig.h"
#include "FTPClientFileState.h"

using namespace nsFTP;

CFTPFileStatus::CFTPFileStatus() :
   m_fTryCwd(false),
   m_fTryRetr(false),
   m_enSizeType(stUnknown),
   m_lSize(-1),
   m_enModificationTimeType(mttUnknown),
   m_mtime(0),
   m_enIDType(idUnknown)
{
}

CFTPFileStatus::CFTPFileStatus(const CFTPFileStatus& src) :
   m_strName(src.m_strName),
   m_strPath(src.m_strPath),
   m_fTryCwd(src.m_fTryCwd),
   m_fTryRetr(src.m_fTryRetr),
   m_enSizeType(src.m_enSizeType),
   m_lSize(src.m_lSize),
   m_enModificationTimeType(src.m_enModificationTimeType),
   m_mtime(src.m_mtime),
   m_strAttributes(src.m_strAttributes),
   m_strUID(src.m_strUID),
   m_strGID(src.m_strGID),
   m_strLink(src.m_strLink),
   m_enIDType(src.m_enIDType),
   m_strID(src.m_strID)
#ifdef _DEBUG
   ,m_strMTime(src.m_strMTime)
#endif
{
}

CFTPFileStatus::~CFTPFileStatus()
{
}

CFTPFileStatus& CFTPFileStatus::operator=(const CFTPFileStatus& rhs)
{
   if( &rhs == this )
      return *this;

   m_strName                = rhs.m_strName;
   m_strPath                = rhs.m_strPath;
   m_fTryCwd                = rhs.m_fTryCwd;
   m_fTryRetr               = rhs.m_fTryRetr;
   m_enSizeType             = rhs.m_enSizeType;
   m_lSize                  = rhs.m_lSize;
   m_enModificationTimeType = rhs.m_enModificationTimeType;
   m_mtime                  = rhs.m_mtime;
   m_strAttributes          = rhs.m_strAttributes;
   m_strUID                 = rhs.m_strUID;
   m_strGID                 = rhs.m_strGID;
   m_strLink                = rhs.m_strLink;
   m_enIDType               = rhs.m_enIDType;
   m_strID                  = rhs.m_strID;
#ifdef _DEBUG
   m_strMTime               = rhs.m_strMTime;
#endif

   return *this;
}

bool CFTPFileStatus::operator==(const CFTPFileStatus& rhs) const
{
   return m_strName                == rhs.m_strName                &&
          m_strPath                == rhs.m_strPath                &&
          m_fTryCwd                == rhs.m_fTryCwd                &&
          m_fTryRetr               == rhs.m_fTryRetr               &&
          m_enSizeType             == rhs.m_enSizeType             &&
          m_lSize                  == rhs.m_lSize                  &&
          m_enModificationTimeType == rhs.m_enModificationTimeType &&
          m_mtime                  == rhs.m_mtime                  &&
          m_strAttributes          == rhs.m_strAttributes          &&
          m_strUID                 == rhs.m_strUID                 &&
          m_strGID                 == rhs.m_strGID                 &&
          m_strLink                == rhs.m_strLink                &&
          m_enIDType               == rhs.m_enIDType               &&
#ifdef _DEBUG
          m_strMTime               == rhs.m_strMTime               &&
#endif
          m_strID                  == rhs.m_strID;
}

bool CFTPFileStatus::operator!=(const CFTPFileStatus& rhs) const
{
   return !operator==(rhs);
}

void CFTPFileStatus::Reset()
{
   m_strName.erase();
   m_strPath.erase();
   m_fTryCwd                = false;
   m_fTryRetr               = false;
   m_enSizeType             = stUnknown;
   m_lSize                  = -1;
   m_enModificationTimeType = mttUnknown;
   m_mtime                  = 0;
   m_strAttributes.erase();
   m_strUID.erase();
   m_strGID.erase();
   m_strLink.erase();
   m_enIDType               = idUnknown;
   m_strID.erase();
#ifdef _DEBUG
   m_strMTime.erase();
#endif
}
