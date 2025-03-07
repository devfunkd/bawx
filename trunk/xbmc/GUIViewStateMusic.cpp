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

#include "GUIViewStateMusic.h"
#include "PlayListPlayer.h"
#include "GUIBaseContainer.h" // for VIEW_TYPE_*
#include "VideoDatabase.h"
#include "GUISettings.h"
#include "AdvancedSettings.h"
#include "Settings.h"
#include "FileItem.h"
#include "Key.h"
#include "Util.h"
#include "LocalizeStrings.h"
#include "utils/log.h"

#include "FileSystem/MusicDatabaseDirectory.h"
#include "FileSystem/VideoDatabaseDirectory.h"
#include "FileSystem/PluginDirectory.h"
#include "FileSystem/ShoutcastDirectory.h"

using namespace DIRECTORY;
using namespace MUSICDATABASEDIRECTORY;

int CGUIViewStateWindowMusic::GetPlaylist()
{
  //return PLAYLIST_MUSIC_TEMP;
  return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusic::UnrollArchives()
{
  return g_guiSettings.GetBool("filelists.unrollarchives");
}

bool CGUIViewStateWindowMusic::AutoPlayNextItem()
{
  return g_guiSettings.GetBool("mymusic.autoplaynextitem");
}

CStdString CGUIViewStateWindowMusic::GetLockType()
{
  return "music";
}

CStdString CGUIViewStateWindowMusic::GetExtensions()
{
  return g_stSettings.m_musicExtensions;
}

CGUIViewStateMusicSearch::CGUIViewStateMusicSearch(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.librarytrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = g_guiSettings.GetString("musicfiles.trackformat");
  CStdString strTrackRight=g_guiSettings.GetString("musicfiles.librarytrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = g_guiSettings.GetString("musicfiles.trackformatright");

  CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
  if (strAlbumLeft.IsEmpty())
    strAlbumLeft = "%B"; // album
  CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
  if (strAlbumRight.IsEmpty())
    strAlbumRight = "%A"; // artist

  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
  {
    AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D", "%L", "%A"));  // Title, Artist, Duration| empty, empty
    SetSortMethod(SORT_METHOD_TITLE_IGNORE_THE);
  }
  else
  {
    AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D", "%L", "%A"));  // Title, Artist, Duration| empty, empty
    SetSortMethod(SORT_METHOD_TITLE);
  }

  SetViewAsControl(g_stSettings.m_viewStateMusicNavSongs.m_viewMode);

  SetSortOrder(g_stSettings.m_viewStateMusicNavSongs.m_sortOrder);

  LoadViewState(items.m_strPath, WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSearch::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV, &g_stSettings.m_viewStateMusicNavSongs);
}

CGUIViewStateMusicDatabase::CGUIViewStateMusicDatabase(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CMusicDatabaseDirectory dir;
  NODE_TYPE NodeType=dir.GetDirectoryChildType(items.m_strPath);

  CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.librarytrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = g_guiSettings.GetString("musicfiles.trackformat");
  CStdString strTrackRight=g_guiSettings.GetString("musicfiles.librarytrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = g_guiSettings.GetString("musicfiles.trackformatright");

  CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
  if (strAlbumLeft.IsEmpty())
    strAlbumLeft = "%B"; // album
  CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
  if (strAlbumRight.IsEmpty())
    strAlbumRight = "%A"; // artist

  CLog::Log(LOGDEBUG,"Album format left  = [%s]", strAlbumLeft.c_str());
  CLog::Log(LOGDEBUG,"Album format right = [%s]", strAlbumRight.c_str());

  switch (NodeType)
  {
  case NODE_TYPE_OVERVIEW:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", "%L", ""));  // Filename, empty | Foldername, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SORT_ORDER_NONE);
    }
    break;
  case NODE_TYPE_TOP100:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", "%L", ""));  // Filename, empty | Foldername, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SORT_ORDER_NONE);
    }
    break;
  case NODE_TYPE_GENRE:
    {
      AddSortMethod(SORT_METHOD_GENRE, 551, LABEL_MASKS("%F", "", "%G", ""));  // Filename, empty | Genre, empty
      SetSortMethod(SORT_METHOD_GENRE);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SORT_ORDER_ASC);
    }
    break;
  case NODE_TYPE_YEAR:
    {
      AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "", "%Y", ""));  // Filename, empty | Year, empty
      SetSortMethod(SORT_METHOD_LABEL);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SORT_ORDER_ASC);
    }
    break;
  case NODE_TYPE_ARTIST:
    {
      if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
      {
        AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 551, LABEL_MASKS("%F", "", "%A", ""));  // Filename, empty | Artist, empty
        SetSortMethod(SORT_METHOD_ARTIST_IGNORE_THE);
      }
      else
      {
        AddSortMethod(SORT_METHOD_ARTIST, 551, LABEL_MASKS("%F", "", "%A", ""));  // Filename, empty | Artist, empty
        SetSortMethod(SORT_METHOD_ARTIST);
      }

      SetViewAsControl(g_stSettings.m_viewStateMusicNavArtists.m_viewMode);

      SetSortOrder(g_stSettings.m_viewStateMusicNavArtists.m_sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_COMPILATIONS:
  case NODE_TYPE_ALBUM:
  case NODE_TYPE_YEAR_ALBUM:
    {
      // album
      if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
        AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      else
        AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined

      // artist
      if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
        AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      else
        AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined

      // year
      AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

      SetSortMethod(g_stSettings.m_viewStateMusicNavAlbums.m_sortMethod);

      SetViewAsControl(g_stSettings.m_viewStateMusicNavAlbums.m_viewMode);

      SetSortOrder(g_stSettings.m_viewStateMusicNavAlbums.m_sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_ADDED:
    {
      AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(g_stSettings.m_viewStateMusicNavAlbums.m_viewMode);

      SetSortOrder(SORT_ORDER_NONE);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
    {
      AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(g_stSettings.m_viewStateMusicNavSongs.m_viewMode);

      SetSortOrder(SORT_ORDER_NONE);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(g_stSettings.m_viewStateMusicNavAlbums.m_viewMode);

      SetSortOrder(SORT_ORDER_NONE);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(g_stSettings.m_viewStateMusicNavAlbums.m_viewMode);

      SetSortOrder(SORT_ORDER_NONE);
    }
    break;
  case NODE_TYPE_ALBUM_TOP100:
    {
      AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SORT_ORDER_NONE);
    }
    break;
  case NODE_TYPE_SINGLES:
    {
      if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
      {
        AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
        AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      }
      else
      {
        AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
        AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      }
      AddSortMethod(SORT_METHOD_DURATION, 555, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
      AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));  // Title - Artist, Rating
      
      SetSortMethod(g_stSettings.m_viewStateMusicNavSongs.m_sortMethod);
      
      SetViewAsControl(g_stSettings.m_viewStateMusicNavSongs.m_viewMode);
      
      SetSortOrder(g_stSettings.m_viewStateMusicNavSongs.m_sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
  case NODE_TYPE_ALBUM_TOP100_SONGS:
  case NODE_TYPE_YEAR_SONG:
  case NODE_TYPE_SONG:
    {
      AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
      if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
      {
        AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
        AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      }
      else
      {
        AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Title, Artist, Duration| empty, empty
        AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
        AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      }
      AddSortMethod(SORT_METHOD_DURATION, 555, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
      AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));  // Title - Artist, Rating

      // the "All Albums" entries always default to SORT_METHOD_ALBUM as this is most logical - user can always
      // change it and the change will be saved for this particular path
      if (dir.IsAllItem(items.m_strPath))
        SetSortMethod(g_guiSettings.GetBool("filelists.ignorethewhensorting") ? SORT_METHOD_ALBUM_IGNORE_THE : SORT_METHOD_ALBUM);
      else
        SetSortMethod(g_stSettings.m_viewStateMusicNavSongs.m_sortMethod);

      SetViewAsControl(g_stSettings.m_viewStateMusicNavSongs.m_viewMode);

      SetSortOrder(g_stSettings.m_viewStateMusicNavSongs.m_sortOrder);
    }
    break;
  case NODE_TYPE_SONG_TOP100:
    {
      AddSortMethod(SORT_METHOD_NONE, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      SetSortMethod(SORT_METHOD_NONE);

      SetViewAsControl(g_stSettings.m_viewStateMusicNavSongs.m_viewMode);

      SetSortOrder(SORT_ORDER_NONE);
    }
    break;
  default:
    break;
  }

  LoadViewState(items.m_strPath, WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicDatabase::SaveViewState()
{
  CMusicDatabaseDirectory dir;
  NODE_TYPE NodeType=dir.GetDirectoryChildType(m_items.m_strPath);

  switch (NodeType)
  {
    case NODE_TYPE_ARTIST:
      SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV, &g_stSettings.m_viewStateMusicNavArtists);
      break;
    case NODE_TYPE_ALBUM_COMPILATIONS:
    case NODE_TYPE_ALBUM:
    case NODE_TYPE_YEAR_ALBUM:
      SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV, &g_stSettings.m_viewStateMusicNavAlbums);
      break;
    case NODE_TYPE_ALBUM_RECENTLY_ADDED:
    case NODE_TYPE_ALBUM_TOP100:
    case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
      SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV);
      break;
    case NODE_TYPE_SINGLES:
    case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
    case NODE_TYPE_SONG:
    case NODE_TYPE_YEAR_SONG:
      SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV, &g_stSettings.m_viewStateMusicNavSongs);
      break;
    case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
    case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
    case NODE_TYPE_SONG_TOP100:
      SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV);
      break;
    default:
      SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV);
      break;
  }
}


CGUIViewStateMusicSmartPlaylist::CGUIViewStateMusicSmartPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
  CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

  AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS(strTrackLeft, strTrackRight));
  AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
  {
    AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
  }
  else
  {
    AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
  }
  AddSortMethod(SORT_METHOD_DURATION, 555, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
  AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));  // Titel, Artist, Rating| empty, empty
  SetSortMethod(g_stSettings.m_viewStateMusicNavSongs.m_sortMethod);

  SetViewAsControl(g_stSettings.m_viewStateMusicNavSongs.m_viewMode);

  SetSortOrder(g_stSettings.m_viewStateMusicNavSongs.m_sortOrder);
  LoadViewState(items.m_strPath, WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSmartPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV, &g_stSettings.m_viewStateMusicNavSongs);
}

CGUIViewStateMusicPlaylist::CGUIViewStateMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
  CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

  AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS(strTrackLeft, strTrackRight));
  AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
  {
    AddSortMethod(SORT_METHOD_TITLE_IGNORE_THE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
  }
  else
  {
    AddSortMethod(SORT_METHOD_TITLE, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
    AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
  }
  AddSortMethod(SORT_METHOD_DURATION, 555, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
  AddSortMethod(SORT_METHOD_SONG_RATING, 563, LABEL_MASKS("%T - %A", "%R"));  // Titel, Artist, Rating| empty, empty

  SetSortMethod(g_stSettings.m_viewStateMusicFiles.m_sortMethod);
  SetViewAsControl(g_stSettings.m_viewStateMusicFiles.m_viewMode);
  SetSortOrder(g_stSettings.m_viewStateMusicFiles.m_sortOrder);

  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

void CGUIViewStateMusicPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES);
}


CGUIViewStateWindowMusicNav::CGUIViewStateWindowMusicNav(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Foldername, empty
    SetSortMethod(SORT_METHOD_NONE);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SORT_ORDER_NONE);
  }
  else
  {
    if (items.IsVideoDb() && items.Size() > (g_guiSettings.GetBool("filelists.hideparentdiritems")?0:1))
    {

      DIRECTORY::VIDEODATABASEDIRECTORY::CQueryParams params;
      DIRECTORY::CVideoDatabaseDirectory::GetQueryParams(items[g_guiSettings.GetBool("filelists.hideparentdiritems")?0:1]->m_strPath,params);
      if (params.GetMVideoId() != -1)
      {
        if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
          AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 556, LABEL_MASKS("%T", "%Y"));  // Filename, Duration | Foldername, empty
        else
          AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%Y"));  // Filename, Duration | Foldername, empty
        AddSortMethod(SORT_METHOD_YEAR,562, LABEL_MASKS("%T", "%Y"));
        if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
        {
          AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE,557, LABEL_MASKS("%A - %T", "%Y"));
          AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE,558, LABEL_MASKS("%B - %T", "%Y"));
        }
        else
        {
          AddSortMethod(SORT_METHOD_ARTIST,557, LABEL_MASKS("%A - %T", "%Y"));
          AddSortMethod(SORT_METHOD_ALBUM,558, LABEL_MASKS("%B - %T", "%Y"));
        }
        CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
        CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");
        AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
      }
      else
      {
        AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
        SetSortMethod(SORT_METHOD_LABEL);
      }
    }
    else
    {
      AddSortMethod(SORT_METHOD_LABEL_WITH_SHARES, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
      SetSortMethod(SORT_METHOD_LABEL_WITH_SHARES);
    }
    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SORT_ORDER_ASC);
  }
  LoadViewState(items.m_strPath, WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::AddOnlineShares()
{
  if (!g_guiSettings.GetBool("network.enableinternet")) return;
  if (!g_advancedSettings.m_bVirtualShares) return;
  for (int i = 0; i < (int)g_settings.m_musicSources.size(); ++i)
  {
    CMediaSource share = g_settings.m_musicSources.at(i);
    if (share.strPath.Find(SHOUTCAST_MASTER_LINK) == 0)//shoutcast shares
      m_sources.push_back(share);
    else if (share.strPath.Find("lastfm://") == 0)//lastfm share
      m_sources.push_back(share);
    }
  }

VECSOURCES& CGUIViewStateWindowMusicNav::GetSources()
{
  //  Setup shares we want to have
  m_sources.clear();
  //  Musicdb shares
  CFileItemList items;
  CDirectory::GetDirectory("musicdb://", items);
  for (int i=0; i<items.Size(); ++i)
  {
    CFileItemPtr item=items[i];
    CMediaSource share;
    share.strName=item->GetLabel();
    share.strPath = item->m_strPath;
      share.m_strThumbnailImage = item->GetThumbnailImage();
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    m_sources.push_back(share);
  }

  //  Playlists share
  CMediaSource share;
  share.strName=g_localizeStrings.Get(136); // Playlists
  share.strPath = "special://musicplaylists/";
  share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicPlaylists.png");
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  AddOnlineShares();

  // Search share
  share.strName=g_localizeStrings.Get(137); // Search
  share.strPath = "musicsearch://";
  share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicSearch.png");
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  // music video share
  CVideoDatabase database;
  database.Open();
  if (database.HasContent(VIDEODB_CONTENT_MUSICVIDEOS))
  {
    share.strName = g_localizeStrings.Get(20389);
    share.strPath = "videodb://3/";
    share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicVideos.png");
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    m_sources.push_back(share);
  }

  // plugins share
  if (CPluginDirectory::HasPlugins("music") && g_advancedSettings.m_bVirtualShares)
  {
    share.strName = g_localizeStrings.Get(1038);
    share.strPath = "plugin://music/";
    share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicPlugins.png");
    share.m_ignore = true;
    m_sources.push_back(share);
  }

  return CGUIViewStateWindowMusic::GetSources();
}

CGUIViewStateWindowMusicSongs::CGUIViewStateWindowMusicSongs(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS()); // Preformated
    AddSortMethod(SORT_METHOD_DRIVE_TYPE, 564, LABEL_MASKS()); // Preformated
    SetSortMethod(SORT_METHOD_LABEL);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SORT_ORDER_ASC);
  }
  else
  {
    CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
    CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

    if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
      AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    else
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I"));  // Userdefined, Size | FolderName, Size
    AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS(strTrackLeft, "%J", "%L", "%J"));  // Userdefined, Date | FolderName, Date
    AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    SetSortMethod(g_stSettings.m_viewStateMusicFiles.m_sortMethod);
    SetViewAsControl(g_stSettings.m_viewStateMusicFiles.m_viewMode);
    SetSortOrder(g_stSettings.m_viewStateMusicFiles.m_sortOrder);
  }
  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

void CGUIViewStateWindowMusicSongs::SaveViewState()
{
    SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES, &g_stSettings.m_viewStateMusicFiles);  
  }

VECSOURCES& CGUIViewStateWindowMusicSongs::GetSources()
{
  bool bIsSourceName = true;
  // plugins share
  if (CPluginDirectory::HasPlugins("music") && g_advancedSettings.m_bVirtualShares)
  {
    CMediaSource share;
    share.strName = g_localizeStrings.Get(1038);
    share.strPath = "plugin://music/";
    if (CUtil::GetMatchingSource(share.strName, g_settings.m_musicSources, bIsSourceName) < 0)
      g_settings.m_musicSources.push_back(share);
  }
  return g_settings.m_musicSources; 
}

CGUIViewStateWindowMusicSongsBoxeeDb::CGUIViewStateWindowMusicSongsBoxeeDb(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SORT_METHOD_LABEL_WITH_SHARES, 551, LABEL_MASKS()); // Preformated
    AddSortMethod(SORT_METHOD_DRIVE_TYPE, 564, LABEL_MASKS()); // Preformated
    SetSortMethod(SORT_METHOD_LABEL_WITH_SHARES);

    SetViewAsControl(g_guiSettings.GetInt("musicfiles.viewmode"));

    SetSortOrder(SORT_ORDER_ASC);
  }
  else
  {
    CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
    CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

    AddSortMethod(SORT_METHOD_LABEL_WITH_SHARES, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    //AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I"));  // Userdefined, Size | FolderName, Size
    AddSortMethod(SORT_METHOD_DATE_WITH_SHARES, 552, LABEL_MASKS(strTrackLeft, "%J", "%L", "%J"));  // Userdefined, Date | FolderName, Date
    AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    SetSortMethod(SORT_METHOD_LABEL_WITH_SHARES);
    SetViewAsControl(g_guiSettings.GetInt("musicfiles.viewmode"));
    SetSortOrder(SORT_ORDER_ASC);
  }
  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

void CGUIViewStateWindowMusicSongsBoxeeDb::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES);
}

VECSOURCES& CGUIViewStateWindowMusicSongsBoxeeDb::GetShares()
{
  return g_settings.m_musicSources;
}

CGUIViewStateWindowMusicPlaylist::CGUIViewStateWindowMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.nowplayingtrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = g_guiSettings.GetString("musicfiles.trackformat");
  CStdString strTrackRight=g_guiSettings.GetString("musicfiles.nowplayingtrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = g_guiSettings.GetString("musicfiles.trackformatright");

  AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  SetSortMethod(SORT_METHOD_NONE);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SORT_ORDER_NONE);

  LoadViewState(items.m_strPath, WINDOW_MUSIC_PLAYLIST);
}

void CGUIViewStateWindowMusicPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_PLAYLIST);
}

int CGUIViewStateWindowMusicPlaylist::GetPlaylist()
{
  return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusicPlaylist::AutoPlayNextItem()
{
  return false;
}

bool CGUIViewStateWindowMusicPlaylist::HideParentDirItems()
{
  return true;
}

VECSOURCES& CGUIViewStateWindowMusicPlaylist::GetSources()
{
  m_sources.clear();
  //  Playlist share
  CMediaSource share;
  share.strPath = "playlistmusic://";
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  return CGUIViewStateWindowMusic::GetSources();
}

CGUIViewStateMusicShoutcast::CGUIViewStateMusicShoutcast(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  /* sadly m_idepth isn't remembered when a directory is retrieved from cache */
  /* and thus this check hardly ever works, so let's just disable it for now */
  if( true || m_items.m_idepth > 1 )
  { /* station list */
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "%B kbps", "%K", ""));  // Title, Bitrate | Title, nothing
    AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%K", "%A listeners", "%K", ""));  // Titel, Listeners | Titel, nothing
    AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%K", "%B kbps", "%K", ""));  // Title, Bitrate | Title, nothing

    SetSortMethod(g_stSettings.m_viewStateMusicShoutcast.m_sortMethod);
    SetSortOrder(g_stSettings.m_viewStateMusicShoutcast.m_sortOrder);
  }
  else
  { /* genre list */
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "", "%K", ""));  // Title, nothing | Title, nothing
    SetSortMethod(SORT_METHOD_LABEL);
    SetSortOrder(SORT_ORDER_ASC); /* maybe we should have this stored somewhere */
  }

  SetViewAsControl(DEFAULT_VIEW_ICONS | 54);
  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

bool CGUIViewStateMusicShoutcast::AutoPlayNextItem()
{
  return false;
}

void CGUIViewStateMusicShoutcast::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES, &g_stSettings.m_viewStateMusicShoutcast);
}

CGUIViewStateMusicShoutcastGenre::CGUIViewStateMusicShoutcastGenre(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  /* sadly m_idepth isn't remembered when a directory is retrieved from cache */
  /* and thus this check hardly ever works, so let's just disable it for now */
  if( true || m_items.m_idepth > 1 )
  { /* station list */
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "%B kbps", "%K", ""));  // Title, Bitrate | Title, nothing
    AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%K", "%A listeners", "%K", ""));  // Titel, Listeners | Titel, nothing
    AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%K", "%B kbps", "%K", ""));  // Title, Bitrate | Title, nothing

    SetSortMethod(g_stSettings.m_viewStateMusicShoutcast.m_sortMethod);
    SetSortOrder(g_stSettings.m_viewStateMusicShoutcast.m_sortOrder);
  }
  else
  { /* genre list */
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "", "%K", ""));  // Title, nothing | Title, nothing
    SetSortMethod(SORT_METHOD_LABEL);
    SetSortOrder(SORT_ORDER_ASC); /* maybe we should have this stored somewhere */
  }

  SetViewAsControl(DEFAULT_VIEW_LIST);
  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

bool CGUIViewStateMusicShoutcastGenre::AutoPlayNextItem()
{
  return false;
}

void CGUIViewStateMusicShoutcastGenre::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES, &g_stSettings.m_viewStateMusicShoutcast);
}

CGUIViewStateMusicLastFM::CGUIViewStateMusicLastFM(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
  CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

  AddSortMethod(SORT_METHOD_UNSORTED, 571, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I"));  // Userdefined, Size | FolderName, Size
  AddSortMethod(SORT_METHOD_LABEL_FILES_FIRST, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  
  
  //SetSortMethod(g_stSettings.m_viewStateMusicLastFM.m_sortMethod);
  SetSortMethod(SORT_METHOD_LABEL_FILES_FIRST);
  SetSortOrder(g_stSettings.m_viewStateMusicLastFM.m_sortOrder);

  SetViewAsControl(DEFAULT_VIEW_LIST);
  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

bool CGUIViewStateMusicLastFM::AutoPlayNextItem()
{
  return false;
}

void CGUIViewStateMusicLastFM::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES, &g_stSettings.m_viewStateMusicLastFM);
}

CGUIViewStateMusicLastFMTags::CGUIViewStateMusicLastFMTags(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
  CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");

  AddSortMethod(SORT_METHOD_UNSORTED, 571, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  AddSortMethod(SORT_METHOD_LABEL_FILES_FIRST, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I"));  // Userdefined, Size | FolderName, Size

  //SetSortMethod(g_stSettings.m_viewStateMusicLastFM.m_sortMethod);
  SetSortMethod(SORT_METHOD_LABEL_FILES_FIRST);
  SetSortOrder(g_stSettings.m_viewStateMusicLastFM.m_sortOrder);

  SetViewAsControl(DEFAULT_VIEW_ICONS | 54);
  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

bool CGUIViewStateMusicLastFMTags::AutoPlayNextItem()
{
  return false;
}

void CGUIViewStateMusicLastFMTags::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES, &g_stSettings.m_viewStateMusicLastFM);
}

CGUIViewStateWindowMusicRssDirectory::CGUIViewStateWindowMusicRssDirectory(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting")) {
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%K", "%I", "%L", ""));  // Titel, Size | Foldername, empty
    SetSortMethod(SORT_METHOD_LABEL_IGNORE_THE);
  }
  else {
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "%I", "%L", ""));  // Titel, Size | Foldername, empty
    SetSortMethod(SORT_METHOD_LABEL);
  }
  
  AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%K", "%J", "%L", "%J"));  // Titel, Date | Foldername, Date
  
  SetSortOrder(SORT_ORDER_ASC);

  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

void CGUIViewStateWindowMusicRssDirectory::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES);
}

CGUIViewStateWindowMusicRssItems::CGUIViewStateWindowMusicRssItems(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%K", "%I", "%L", ""));  // Titel, Size | Foldername, empty
  else
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%K", "%I", "%L", ""));  // Titel, Size | Foldername, empty
  AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%K", "%J", "%L", "%J"));  // Titel, Date | Foldername, Date

  SetSortMethod(SORT_METHOD_DATE);
  SetSortOrder(SORT_ORDER_ASC);

  LoadViewState(items.m_strPath, WINDOW_MUSIC_FILES);
}

void CGUIViewStateWindowMusicRssItems::SaveViewState()
{
  SaveViewToDb(m_items.m_strPath, WINDOW_MUSIC_FILES);
}

