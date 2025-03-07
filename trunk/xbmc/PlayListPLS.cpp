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

#include "PlayListPLS.h"
#include "PlayListFactory.h"
#include "Util.h"
#include "StringUtils.h"
#include "FileSystem/File.h"
#include "AdvancedSettings.h"
#include "MusicInfoTag.h"
#include "utils/CharsetConverter.h"
#include "utils/log.h"
#include "tinyXML/tinyxml.h"

using namespace std;
using namespace XFILE;
using namespace PLAYLIST;

#define START_PLAYLIST_MARKER "[playlist]"
#define PLAYLIST_NAME     "PlaylistName"

/*----------------------------------------------------------------------
[playlist]
PlaylistName=Playlist 001
File1=E:\Program Files\Winamp3\demo.mp3
Title1=demo
Length1=5
File2=E:\Program Files\Winamp3\demo.mp3
Title2=demo
Length2=5
NumberOfEntries=2
Version=2
----------------------------------------------------------------------*/
CPlayListPLS::CPlayListPLS(void)
{}

CPlayListPLS::~CPlayListPLS(void)
{}

bool CPlayListPLS::Load(const CStdString &strFile)
{
  //read it from the file
  CStdString strFileName(strFile);
  m_strPlayListName = CUtil::GetFileName(strFileName);

  Clear();

  bool bShoutCast = false;
  if( strFileName.Left(8).Equals("shout://") )
  {
    strFileName.Delete(0, 8);
    strFileName.Insert(0, "http://");
    m_strBasePath = "";
    bShoutCast = true;
  }
  else
    CUtil::GetParentPath(strFileName, m_strBasePath);

  CFile file;
  if (!file.Open(strFileName) )
  {
    file.Close();
    return false;
  }

  if (file.GetLength() > 1024*1024)
  {
    CLog::Log(LOGWARNING, "%s - File is larger than 1 MB, most likely not a playlist",__FUNCTION__);
    return false;
  }

  char szLine[4096];
  CStdString strLine;

  // run through looking for the [playlist] marker.
  // if we find another http stream, then load it.
  while (1)
  {    
    if ( !file.ReadString(szLine, sizeof(szLine) ) )
    {
      file.Close();
      return size() > 0;
    }
    strLine = szLine;
    strLine.TrimLeft(" \t");
    strLine.TrimRight(" \n\r");
    if(strLine.Equals(START_PLAYLIST_MARKER))
      break;

    // if there is something else before playlist marker, this isn't a pls file
    if(!strLine.IsEmpty())
      return false;
  }

  while (file.ReadString(szLine, sizeof(szLine) ) )
  {
    strLine = szLine;
    StringUtils::RemoveCRLF(strLine);
    int iPosEqual = strLine.Find("=");
    if (iPosEqual > 0)
    {
      CStdString strLeft = strLine.Left(iPosEqual);
      iPosEqual++;
      CStdString strValue = strLine.Right(strLine.size() - iPosEqual);
      strLeft.ToLower();
      while (strLeft[0] == ' ' || strLeft[0] == '\t')
        strLeft.erase(0,1);

      if (strLeft == "numberofentries")
      {
        m_vecItems.reserve(atoi(strValue.c_str()));
      }
      else if (strLeft.Left(4) == "file")
      {
        vector <int>::size_type idx = atoi(strLeft.c_str() + 4);
        Resize(idx);

        if (idx == 0)
        {
          CLog::Log(LOGWARNING, "%s - Not a valid PLS playlist, location of first file is not permitted (File0 should be File1)",__FUNCTION__);
          return false;
        }

        if (m_vecItems[idx - 1]->GetLabel().empty())
          m_vecItems[idx - 1]->SetLabel(CUtil::GetFileName(strValue));
        CFileItem item(strValue, false);
        if (bShoutCast && !item.IsAudio())
          strValue.Replace("http:", "shout:");

        if (CUtil::IsRemote(m_strBasePath) && g_advancedSettings.m_pathSubstitutions.size() > 0)
         strValue = CUtil::SubstitutePath(strValue);
        CUtil::GetQualifiedFilename(m_strBasePath, strValue);
        g_charsetConverter.unknownToUTF8(strValue);
        m_vecItems[idx - 1]->m_strPath = strValue;
      }
      else if (strLeft.Left(5) == "title")
      {
        vector <int>::size_type idx = atoi(strLeft.c_str() + 5);
        Resize(idx);
        g_charsetConverter.unknownToUTF8(strValue);
        m_vecItems[idx - 1]->SetLabel(strValue);
      }
      else if (strLeft.Left(6) == "length")
      {
        vector <int>::size_type idx = atoi(strLeft.c_str() + 6);
        Resize(idx);
        m_vecItems[idx - 1]->GetMusicInfoTag()->SetDuration(atol(strValue.c_str()));
        }
      else if (strLeft == "playlistname")
      {
        m_strPlayListName = strValue;
        g_charsetConverter.unknownToUTF8(m_strPlayListName);
      }
    }
  }
  file.Close();

  // check for missing entries
  ivecItems p = m_vecItems.begin();
  while ( p != m_vecItems.end())
  {
    if ((*p)->m_strPath.empty())
    {
      p = m_vecItems.erase(p);
    }
    else
    {
       ++p;
    }
  }

  return true;
}

void CPlayListPLS::Save(const CStdString& strFileName) const
{
  if (!m_vecItems.size()) return ;
  CStdString strPlaylist = CUtil::MakeLegalPath(strFileName);
  CFile file;
  if (!file.OpenForWrite(strPlaylist, true))
  {
    CLog::Log(LOGERROR, "Could not save PLS playlist: [%s]", strPlaylist.c_str());
    return ;
  }
  CStdString write;
  write.AppendFormat("%s\n", START_PLAYLIST_MARKER);
  CStdString strPlayListName=m_strPlayListName;
  g_charsetConverter.utf8ToStringCharset(strPlayListName);
  write.AppendFormat("PlaylistName=%s\n", strPlayListName.c_str() );

  for (int i = 0; i < (int)m_vecItems.size(); ++i)
  {
    CFileItemPtr item = m_vecItems[i];
    CStdString strFileName=item->m_strPath;
    g_charsetConverter.utf8ToStringCharset(strFileName);
    CStdString strDescription=item->GetLabel();
    g_charsetConverter.utf8ToStringCharset(strDescription);
    write.AppendFormat("File%i=%s\n", i + 1, strFileName.c_str() );
    write.AppendFormat("Title%i=%s\n", i + 1, strDescription.c_str() );
    write.AppendFormat("Length%i=%u\n", i + 1, item->GetMusicInfoTag()->GetDuration() / 1000 );
  }

  write.AppendFormat("NumberOfEntries=%i\n", m_vecItems.size());
  write.AppendFormat("Version=2\n");
  file.Write(write.c_str(), write.size());
  file.Close();
}

bool CPlayListASX::LoadAsxIniInfo(istream &stream)
{
  CLog::Log(LOGINFO, "Parsing INI style ASX");

  string name, value;

  while( stream.good() )
  {
    // consume blank rows, and blanks
    while((stream.peek() == '\r' || stream.peek() == '\n' || stream.peek() == ' ') && stream.good())
      stream.get();

    if(stream.peek() == '[')
    {
      // this is an [section] part, just ignore it
      while(stream.good() && stream.peek() != '\r' && stream.peek() != '\n')
        stream.get();
      continue;
    }
    name = "";
    value = "";
    // consume name
    while(stream.peek() != '\r' && stream.peek() != '\n' && stream.peek() != '=' && stream.good())
      name += stream.get();

    // consume =
    if(stream.get() != '=')
      continue;

    // consume value
    while(stream.peek() != '\r' && stream.peek() != '\n' && stream.good())
      value += stream.get();

    CLog::Log(LOGINFO, "Adding element %s=%s", name.c_str(), value.c_str());
    CFileItemPtr newItem(new CFileItem(value));
    newItem->m_strPath = value;
    Add(newItem);
  }

  return true;
}

bool CPlayListASX::LoadData(istream& stream)
{
  CLog::Log(LOGNOTICE, "Parsing ASX");

  if(stream.peek() == '[')
  {
    return LoadAsxIniInfo(stream);
  }
  else if(stream.peek() == 'm' || stream.peek() == 'M' || 
          stream.peek() == 'h' || stream.peek() == 'H') // ugly hack - some playlists just hold links separated by ';' (castup shenanigans)
  {
    CStdString strLinks;
    stream >> strLinks;
    std::vector<CStdString> links;
    CUtil::Tokenize(strLinks, links, ";");
    if (links.size())
    {
      CFileItemPtr newItem(new CFileItem(links[0]));
      newItem->m_strPath = links[0];
      for (size_t i=1; i< links.size(); i++)
        newItem->m_fallbackPaths.push_back(links[i]);
      Add(newItem);
    }    
    return (links.size() > 0);
  }
  else
  {
    TiXmlDocument xmlDoc;
    stream >> xmlDoc;

    if (xmlDoc.Error())
    {
      CLog::Log(LOGERROR, "Unable to parse ASX info Error: %s", xmlDoc.ErrorDesc());
      return false;
    }

    TiXmlElement *pRootElement = xmlDoc.RootElement();

    // lowercase every element
    TiXmlNode *pNode = pRootElement;
    TiXmlNode *pChild = NULL;
    CStdString value;
    value = pNode->Value();
    value.ToLower();
    pNode->SetValue(value);
    while(pNode)
    {
      pChild = pNode->IterateChildren(pChild);
      if(pChild)
      {
        if (pChild->Type() == TiXmlNode::ELEMENT)
        {
          value = pChild->Value();
          value.ToLower();
          pChild->SetValue(value);

          TiXmlAttribute* pAttr = pChild->ToElement()->FirstAttribute();
          while(pAttr)
          {
            value = pAttr->Name();
            value.ToLower();
            pAttr->SetName(value);
            pAttr = pAttr->Next();
          }
        }

        pNode = pChild;
        pChild = NULL;
        continue;
      }

      pChild = pNode;
      pNode = pNode->Parent();
    }
    CStdString roottitle = "";
    TiXmlElement *pElement = pRootElement->FirstChildElement();
    while (pElement)
    {
      value = pElement->Value();
      if (value == "title")
      {
        roottitle = pElement->GetText();
      }
      else if (value == "entry")
      {
        CStdString title(roottitle);

        TiXmlElement *pRef = pElement->FirstChildElement("ref");
        TiXmlElement *pTitle = pElement->FirstChildElement("title");

        if(pTitle)
          title = pTitle->GetText();

        CFileItemPtr newItem(new CFileItem(title));
        while (pRef)
        { // multiple references may apear for one entry
          // duration may exist on this level too
          value = pRef->Attribute("href");
          if (value != "")
          {
            if(title.IsEmpty())
            {
              title = value;
              newItem->SetLabel(title);
            }

            if (newItem->m_strPath.IsEmpty())
            {
            CLog::Log(LOGINFO, "Adding element %s, %s", title.c_str(), value.c_str());
            newItem->m_strPath = value;
          }
            else
            {
              CLog::Log(LOGINFO, "Adding fallback path %s, %s", title.c_str(), value.c_str());
              newItem->m_fallbackPaths.push_back(value);
            }            
          }
          pRef = pRef->NextSiblingElement("ref");
        }
        
        if (!newItem->m_strPath.IsEmpty())
        {
          Add(newItem);
      }
      }
      else if (value == "entryref")
      {
        value = pElement->Attribute("href");
        if (value != "")
        { // found an entryref, let's try loading that url
          auto_ptr<CPlayList> playlist(CPlayListFactory::Create(value));
          if (NULL != playlist.get())
            if (playlist->Load(value))
              Add(*playlist);
        }
      }
      pElement = pElement->NextSiblingElement();
    }
  }

  return true;
}


bool CPlayListRAM::LoadData(istream& stream)
{
  CLog::Log(LOGINFO, "Parsing RAM");
  
  CStdString strMMS;
  while( stream.peek() != '\n' && stream.peek() != '\r' )
    strMMS += stream.get();
  
  CLog::Log(LOGINFO, "Adding element %s", strMMS.c_str());
  CFileItemPtr newItem(new CFileItem(strMMS));
  newItem->m_strPath = strMMS;
  Add(newItem);
  return true;
}

void CPlayListPLS::Resize(vector <int>::size_type newSize)
{
  while (m_vecItems.size() < newSize)
  {
    CFileItemPtr fileItem(new CFileItem());
    m_vecItems.push_back(fileItem);
  }
}
