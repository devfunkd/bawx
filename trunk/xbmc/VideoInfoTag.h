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


#include <vector>
#include "utils/ScraperUrl.h"
#include "utils/Fanart.h"
#include "utils/StreamDetails.h"

class CArchive;
class TiXmlNode;
class TiXmlElement;

struct SActorInfo
{
  CStdString strName;
  CStdString strRole;
  CScraperUrl thumbUrl;
};

struct SRottenTomatoInfo
{
  SRottenTomatoInfo()
  {
    iCriticsScore = -1;
    iAudienceScore = -1;
    bValidCriticsDetails = false;
    bValidAudienceDetails = false;
  }

  int iCriticsScore;
  int iAudienceScore;
  CStdString strCriticsRating;
  CStdString strAudienceRating;
  bool       bValidCriticsDetails;
  bool       bValidAudienceDetails;
};

class CVideoInfoTag : public IArchivable, public ISerializable
{
public:
  CVideoInfoTag() { Reset(); };
  void Reset();
  bool Load(const TiXmlElement *movie, bool chained = false);
  bool Save(TiXmlNode *node, const CStdString &tag, bool savePathInfo = true);
  virtual void Archive(CArchive& ar);
  virtual void Serialize(CVariant& value);
  const CStdString GetCast(bool bIncludeRole = false) const;
  bool HasStreamDetails() const;
  bool IsEmpty() const;
  void MergeFieldsFrom(const CVideoInfoTag& other);

  void Dump() const;

  CStdString m_strDirector;
  CStdString m_strWritingCredits;
  CStdString m_strGenre;
  CStdString m_strTagLine;
  CStdString m_strPlotOutline;
  CStdString m_strTrailer;
  CStdString m_strPlot;
  CScraperUrl m_strPictureURL;
  CStdString m_strCountry;
  CStdString m_strShowLink;
  CStdString m_strTitle;
  CStdString m_strSortTitle;
  CStdString m_strVotes;
  CStdString m_strArtist;
  std::vector< SActorInfo > m_cast;
  typedef std::vector< SActorInfo >::const_iterator iCast;
  CStdString m_strSet;
  CStdString m_strRuntime;
  CStdString m_strFile;
  CStdString m_strPath;
  CStdString m_strIMDBNumber;
  CStdString m_strMPAARating;
  CStdString m_strFileNameAndPath;
  CStdString m_strOriginalTitle;
  CStdString m_strEpisodeGuide;
  CStdString m_strPremiered;
  CStdString m_strStatus;
  CStdString m_strProductionCode;
  CStdString m_strFirstAired;
  CStdString m_strShowTitle;
  CStdString m_strStudio;
  CStdString m_strAlbum;
  CStdString m_lastPlayed;
  int m_playCount;
  int m_iTop250;
  int m_iYear;
  int m_iSeason;
  int m_iEpisode;
  int m_iDbId;
  int m_iFileId; 
  int m_iSpecialSortSeason;
  int m_iSpecialSortEpisode;
  int m_iTrack;
  float m_fRating;
  int m_iRating; // integer value, used for star rating
  int m_iPopularity;
  int m_iBookmarkId;
  CFanart m_fanart;
  CStreamDetails m_streamDetails;
  CStdString  m_strNFOPath;
  int         m_iNfoAccessTime;
  int         m_iNfoModifiedTime;
  SRottenTomatoInfo m_rottenTomatoDetails;
private:
  void ParseNative(const TiXmlElement* movie);
  void ParseMyMovies(const TiXmlElement* movie);
};

typedef std::vector<CVideoInfoTag> VECMOVIES;
