#pragma once
/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
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

#include "StdString.h"

#include <boost/shared_ptr.hpp>

class CFileItem; typedef boost::shared_ptr<CFileItem> CFileItemPtr;
class CFileItemList;
namespace PLAYLIST
{
  class CPlayList;
}

typedef enum
{
  PARTYMODECONTEXT_UNKNOWN,
  PARTYMODECONTEXT_MUSIC,
  PARTYMODECONTEXT_VIDEO
} PartyModeContext;

class CPartyModeManager
{
public:
  CPartyModeManager(void);
  virtual ~CPartyModeManager(void);

  bool Enable(PartyModeContext context=PARTYMODECONTEXT_MUSIC, const CStdString& strXspPath = "");
  void Disable();
  void Play(int iPos);
  void OnSongChange(bool bUpdatePlayed = false);
  void AddUserSongs(PLAYLIST::CPlayList& tempList, bool bPlay = false);
  void AddUserSongs(CFileItemList& tempList, bool bPlay = false);
  bool IsEnabled(PartyModeContext context=PARTYMODECONTEXT_UNKNOWN) const;
  int GetSongsPlayed();
  int GetMatchingSongs();
  int GetMatchingSongsPicked();
  int GetMatchingSongsLeft();
  int GetRelaxedSongs();
  int GetRandomSongs();

private:
  void Process();
  bool AddRandomSongs(int iSongs = 0);
  bool AddInitialSongs(std::vector< std::pair<int,int> > &songIDs);
  void Add(const CFileItemPtr &pItem);
  bool ReapSongs();
  bool MovePlaying();
  void SendUpdateMessage();
  void OnError(int iError, const CStdString& strLogMessage);
  void ClearState();
  void UpdateStats();
  std::pair<CStdString,CStdString> GetWhereClauseWithHistory() const;
  void AddToHistory(int type, int songID);
  void GetRandomSelection(std::vector< std::pair<int,int> > &in, unsigned int number, std::vector< std::pair<int, int> > &out);

  // state
  bool m_bEnabled;
  bool m_bIsVideo;
  int m_iLastUserSong;
  CStdString m_strCurrentFilterMusic;
  CStdString m_strCurrentFilterVideo;
  CStdString m_type;

  // statistics
  int m_iSongsPlayed;
  int m_iMatchingSongs;
  int m_iMatchingSongsPicked;
  int m_iMatchingSongsLeft;
  int m_iRelaxedSongs;
  int m_iRandomSongs;

  // history
  unsigned int m_songsInHistory;
  std::vector< std::pair<int,int> > m_history;
};

extern CPartyModeManager g_partyModeManager;
