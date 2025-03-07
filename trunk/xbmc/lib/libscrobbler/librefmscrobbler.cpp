/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "librefmscrobbler.h"
#include "Application.h"
#include "Atomics.h"
#include "GUISettings.h"
#include "Settings.h"
#include "Util.h"
#include "LocalizeStrings.h"

#ifndef __arm__
long CLibrefmScrobbler::m_instanceLock = 0;
#endif
CLibrefmScrobbler *CLibrefmScrobbler::m_pInstance = NULL;

CLibrefmScrobbler::CLibrefmScrobbler()
  : CScrobbler(LIBREFM_SCROBBLER_HANDSHAKE_URL, LIBREFM_SCROBBLER_LOG_PREFIX)
{
}

CLibrefmScrobbler::~CLibrefmScrobbler()
{
  Term();
}

CLibrefmScrobbler *CLibrefmScrobbler::GetInstance()
{
  if (!m_pInstance) // Avoid spinning aimlessly
  {
#ifndef __arm__
    CAtomicSpinLock lock(m_instanceLock);
#endif
    if (!m_pInstance)
    {
      m_pInstance = new CLibrefmScrobbler;
    }
  }
  return m_pInstance;
}

void CLibrefmScrobbler::RemoveInstance()
{
  if (m_pInstance)
  {
#ifndef __arm__
    CAtomicSpinLock lock(m_instanceLock);
#endif
    delete m_pInstance;
    m_pInstance = NULL;
  }
}

void CLibrefmScrobbler::LoadCredentials()
{
  SetUsername(g_guiSettings.GetString("scrobbler.librefmusername"));
  SetPassword(g_guiSettings.GetString("scrobbler.librefmpassword"));
}

CStdString CLibrefmScrobbler::GetJournalFileName()
{
  CStdString strFileName = g_settings.GetProfileUserDataFolder();
  return CUtil::AddFileToFolder(strFileName, "LibrefmScrobbler.xml");
}

void CLibrefmScrobbler::NotifyUser(int error)
{
  CStdString strText;
  CStdString strAudioScrobbler;
  switch (error)
  {
    case SCROBBLER_USER_ERROR_BADAUTH:
      strText = g_localizeStrings.Get(15206);
      m_bBadAuth = true;
      strAudioScrobbler = g_localizeStrings.Get(15220);  // Libre.fm
      g_application.m_guiDialogKaiToast.QueueNotification("", strAudioScrobbler, strText, 10000);
      break;
    case SCROBBLER_USER_ERROR_BANNED:
      strText = g_localizeStrings.Get(15205);
      m_bBanned = true;
      strAudioScrobbler = g_localizeStrings.Get(15220);  // Libre.fm
      g_application.m_guiDialogKaiToast.QueueNotification("", strAudioScrobbler, strText, 10000);
      break;
    default:
      break;
  }
}

bool CLibrefmScrobbler::CanScrobble()
{
  return (!g_guiSettings.GetString("scrobbler.librefmusername").IsEmpty()  &&
          !g_guiSettings.GetString("scrobbler.librefmpassword").IsEmpty()  &&
         g_guiSettings.GetBool("scrobbler.librefmsubmit"));
}

