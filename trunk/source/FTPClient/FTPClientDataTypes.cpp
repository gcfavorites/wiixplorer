
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
#include "FTPClientDataTypes.h"

using namespace nsFTP;

/// returns the string which is used for display
tstring CFirewallType::AsDisplayString() const
{
   switch( m_enFirewallType )
   {
   case ftNone:                              return _T("no firewall");
   case ftSiteHostName:                      return _T("SITE hostname");
   case ftUserAfterLogon:                    return _T("USER after logon");
   case ftProxyOpen:                         return _T("proxy OPEN");
   case ftTransparent:                       return _T("Transparent");
   case ftUserWithNoLogon:                   return _T("USER with no logon");
   case ftUserFireIDatRemotehost:            return _T("USER fireID@remotehost");
   case ftUserRemoteIDatRemoteHostFireID:    return _T("USER remoteID@remotehost fireID");
   case ftUserRemoteIDatFireIDatRemoteHost:  return _T("USER remoteID@fireID@remotehost");
   }
   ASSERT( false );
   return _T("");
}

/// return the string which is used for storage (e.g. in an XML- or INI-file)
tstring CFirewallType::AsStorageString() const
{
   switch( m_enFirewallType )
   {
   case ftNone:                              return _T("NO_FIREWALL");
   case ftSiteHostName:                      return _T("SITE_HOSTNAME");
   case ftUserAfterLogon:                    return _T("USER_AFTER_LOGON");
   case ftProxyOpen:                         return _T("PROXY_OPEN");
   case ftTransparent:                       return _T("TRANSPARENT");
   case ftUserWithNoLogon:                   return _T("USER_WITH_NO_LOGON");
   case ftUserFireIDatRemotehost:            return _T("USER_FIREID@REMOTEHOST");
   case ftUserRemoteIDatRemoteHostFireID:    return _T("USER_REMOTEID@REMOTEHOST_FIREID");
   case ftUserRemoteIDatFireIDatRemoteHost:  return _T("USER_REMOTEID@FIREID@REMOTEHOST");
   }
   ASSERT( false );
   return _T("");
}

/// returns all available firewall types
void CFirewallType::GetAllTypes(TFirewallTypeVector& vTypes)
{
   vTypes.resize(9);
   vTypes[0] = ftNone;
   vTypes[1] = ftSiteHostName;
   vTypes[2] = ftUserAfterLogon;
   vTypes[3] = ftProxyOpen;
   vTypes[4] = ftTransparent;
   vTypes[5] = ftUserWithNoLogon;
   vTypes[6] = ftUserFireIDatRemotehost;
   vTypes[7] = ftUserRemoteIDatRemoteHostFireID;
   vTypes[8] = ftUserRemoteIDatFireIDatRemoteHost;
}

CLogonInfo::CLogonInfo() :
   m_ushHostport(DEFAULT_FTP_PORT),
   m_strUsername(ANONYMOUS_USER),
   m_ushFwPort(DEFAULT_FTP_PORT),
   m_FwType(CFirewallType::None())
{
}

CLogonInfo::CLogonInfo(const tstring& strHostname, unsigned int ushHostport, const tstring& strUsername, 
                       const tstring& strPassword, unsigned int feat, const tstring& strAccount) :
   m_strHostname(strHostname),
   m_ushHostport(ushHostport),
   m_strUsername(strUsername),
   m_strPassword(strPassword),
   m_feat(feat),
   m_strAccount(strAccount),
   m_ushFwPort(DEFAULT_FTP_PORT),
   m_FwType(CFirewallType::None())
{
}

CLogonInfo::CLogonInfo(const tstring& strHostname, unsigned int ushHostport, const tstring& strUsername, const tstring& strPassword,
					   unsigned int feat, 
                       const tstring& strAccount, const tstring& strFwHostname, const tstring& strFwUsername, 
                       const tstring& strFwPassword, unsigned int ushFwPort, const CFirewallType& crFwType) :
   m_strHostname(strHostname),
   m_ushHostport(ushHostport),
   m_strUsername(strUsername),
   m_strPassword(strPassword),
   m_feat(feat),
   m_strAccount(strAccount),
   m_strFwHostname(strFwHostname),
   m_strFwUsername(strFwUsername),
   m_strFwPassword(strFwPassword),
   m_ushFwPort(ushFwPort),
   m_FwType(crFwType)
{
}

void CLogonInfo::SetHost(const tstring& strHostname, unsigned int ushHostport, const tstring& strUsername, 
                         const tstring& strPassword, const tstring& strAccount)
{
   m_strHostname  = strHostname;
   m_ushHostport  = ushHostport;
   m_strUsername  = strUsername;
   m_strPassword  = strPassword;
   m_strAccount   = strAccount;
}

void CLogonInfo::SetFirewall(const tstring& strFwHostname, const tstring& strFwUsername, const tstring& strFwPassword,
                             unsigned int ushFwPort, const CFirewallType& crFwType)
{
   m_strFwHostname   = strFwHostname;
   m_strFwUsername   = strFwUsername;
   m_strFwPassword   = strFwPassword;
   m_ushFwPort       = ushFwPort;
   m_FwType          = crFwType;
}
