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

#include "GUIFontManager.h"
#include "GraphicContext.h"
#include "GUIWindowManager.h"
#include "SkinInfo.h"
#include "GUIFontTTF.h"
#include "GUIFont.h"
#include "XMLUtils.h"
#include "GUIControlFactory.h"
#include "../xbmc/Util.h"
#include "../xbmc/FileSystem/File.h"
#include "../xbmc/FileSystem/SpecialProtocol.h"
#include "utils/log.h"
#include "WindowingFactory.h"
#include "Application.h"

using namespace std;

GUIFontManager g_fontManager;

GUIFontManager::GUIFontManager(void)
{
  m_skinResolution=RES_INVALID;
  m_fontsetUnicode=false;
  m_canReload = true;
}

GUIFontManager::~GUIFontManager(void)
{
  Clear();
}

CGUIFont* GUIFontManager::LoadTTF(const CStdString& strFontName, const CStdString& strFilename, color_t textColor, color_t shadowColor, const int iSize, const int iStyle, float lineSpacing, float aspect, RESOLUTION sourceRes)
{
  float originalAspect = aspect;
  
  //check if font already exists
  CGUIFont* pFont = GetFont(strFontName, false);
  if (pFont)
    return pFont;

  if (sourceRes == RES_INVALID) // no source res specified, so assume the skin res
    sourceRes = m_skinResolution;


  // set scaling resolution so that we can scale our font sizes correctly
  // as fonts aren't scaled at render time (due to aliasing) we must scale
  // the size of the fonts before they are drawn to bitmaps

  // adjust aspect ratio
  if (sourceRes == RES_PAL_16x9 || sourceRes == RES_PAL60_16x9 || sourceRes == RES_NTSC_16x9 || sourceRes == RES_HDTV_480p_16x9)
    aspect *= 0.75f;

  aspect *= g_graphicsContext.GetGUIScaleY() / g_graphicsContext.GetGUIScaleX();
  float newSize = (float) iSize / g_graphicsContext.GetGUIScaleY();
  
  // First try to load the font from the skin
  CStdString strPath;
  if (!CURI::IsFullPath(strFilename))
  {
    strPath = CUtil::AddFileToFolder(g_graphicsContext.GetMediaDir(), "fonts");
    strPath = CUtil::AddFileToFolder(strPath, strFilename);
  }
  else
    strPath = strFilename;

#ifdef _LINUX
  strPath = PTH_IC(strPath);
#endif

  // Check if the file exists, otherwise try loading it from the global media dir
  if (!XFILE::CFile::Exists(strPath))
  {
    strPath = CUtil::AddFileToFolder("special://xbmc/media/Fonts", CUtil::GetFileName(strFilename));
#ifdef _LINUX
    strPath = PTH_IC(strPath);
#endif
  }
  
#ifdef __APPLE__
  // If the font is not stored in the skin path nor in the media path, see if it's a system font.
  if (!XFILE::CFile::Exists(strPath) && strFilename[1] != ':')
  {
    CStdString fontPath;
    fontPath += "/System/Library/Fonts/" + CUtil::GetFileName(strFilename);
    if (CUtil::GetExtension(strFilename).length() == 0)
      fontPath += ".ttf";
    
    if (XFILE::CFile::Exists(fontPath))
      strPath = fontPath;
    else
    {
      fontPath = "/Library/Fonts/" + CUtil::GetFileName(strFilename);
      if (CUtil::GetExtension(strFilename).length() == 0)
        fontPath += ".ttf";
      if (XFILE::CFile::Exists(fontPath))
        strPath = fontPath;
    }
  }
#endif
  
  // check if we already have this font file loaded (font object could differ only by color or style)
  CStdString TTFfontName;
  TTFfontName.Format("%s_%f_%f", strFilename, newSize, aspect);

  CGUIFontTTFBase* pFontFile = GetFontFile(TTFfontName);
  if (!pFontFile)
  {
    pFontFile = new CGUIFontTTF(TTFfontName);
    bool bFontLoaded = pFontFile->Load(strPath, newSize, aspect);

    if (!bFontLoaded)
    {
      delete pFontFile;

      // font could not be loaded - try boxee.ttf, which we distribute
      if (strFilename != "boxee.ttf")
      {
        CLog::Log(LOGERROR, "Couldn't load font name: %s(%s), trying to substitute boxee.ttf", strFontName.c_str(), strFilename.c_str());
        return LoadTTF(strFontName, "boxee.ttf", textColor, shadowColor, iSize, iStyle, lineSpacing, originalAspect);
      }
      CLog::Log(LOGERROR, "Couldn't load font name:%s file:%s", strFontName.c_str(), strPath.c_str());

      return NULL;
    }
    
    m_vecFontFiles.push_back(pFontFile);
  }

  // font file is loaded, create our CGUIFont
  CGUIFont *pNewFont = new CGUIFont(strFontName, iStyle, textColor, shadowColor, lineSpacing, pFontFile);
  m_vecFonts.push_back(pNewFont);
  
  // Store the original TTF font info in case we need to reload it in a different resolution
  OrigFontInfo fontInfo;
  fontInfo.size = iSize;
  fontInfo.aspect = originalAspect;
  fontInfo.fontFilePath = strPath;
  fontInfo.fileName = strFilename;
  fontInfo.sourceRes = sourceRes;
  m_vecFontInfo.push_back(fontInfo);      
  
  return pNewFont;
}

bool GUIFontManager::OnMessage(CGUIMessage &message)
{
  if (message.GetMessage() != GUI_MSG_NOTIFY_ALL)
    return false;

  if (message.GetParam1() == GUI_MSG_RENDERER_LOST)
  {
    m_canReload = false;
    return true;
  }

  if (message.GetParam1() == GUI_MSG_RENDERER_RESET)
  { // our device has been reset - we have to reload our ttf fonts, and send
    // a message to controls that we have done so
    ReloadTTFFonts();
    g_windowManager.SendMessage(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_WINDOW_RESIZE);
    m_canReload = true;
    return true;
  }
  
  if (message.GetParam1() == GUI_MSG_WINDOW_RESIZE)
  { // we need to reload our fonts
    if (m_canReload)
    {
      ReloadTTFFonts();
      // no need to send a resize message, as this message will do the rounds
      return true;
    }
  }
  return false;
}

void GUIFontManager::ReloadTTFFonts(void)
{
  if (!m_vecFonts.size())
    return;   // we haven't even loaded fonts in yet

  if(g_application.m_pPlayer && g_application.m_pPlayer->IsPlaying())
    return;

  RESOLUTION sourceRes;

  for (unsigned int i = 0; i < m_vecFonts.size(); i++)
  {
    CGUIFont* font = m_vecFonts[i];
    OrigFontInfo fontInfo = m_vecFontInfo[i];      
    
    float aspect = fontInfo.aspect;
    int iSize = fontInfo.size;
    CStdString& strPath = fontInfo.fontFilePath;
    CStdString& strFilename = fontInfo.fileName;
    sourceRes = fontInfo.sourceRes;

    if (fontInfo.sourceRes == RES_PAL_16x9 || fontInfo.sourceRes == RES_PAL60_16x9 || fontInfo.sourceRes == RES_NTSC_16x9 || fontInfo.sourceRes == RES_HDTV_480p_16x9)
      aspect *= 0.75f;
  
    aspect *= g_graphicsContext.GetGUIScaleY() / g_graphicsContext.GetGUIScaleX();
    float newSize = (float) iSize / g_graphicsContext.GetGUIScaleY();

    CStdString TTFfontName;
    TTFfontName.Format("%s_%f_%f", strFilename, newSize, aspect);
    CGUIFontTTFBase* pFontFile = GetFontFile(TTFfontName);
    if (!pFontFile)
    {
      pFontFile = new CGUIFontTTF(TTFfontName);
      if (!pFontFile || !pFontFile->Load(strPath, newSize, aspect))
      {
        delete pFontFile;   
        // font could not be loaded
        CLog::Log(LOGERROR, "Couldn't re-load font file:%s", strPath.c_str());
        return;
      }
  
      m_vecFontFiles.push_back(pFontFile);
    }
          
    font->SetFont(pFontFile);
  }
}

void GUIFontManager::Unload(const CStdString& strFontName)
{
  for (vector<CGUIFont*>::iterator iFont = m_vecFonts.begin(); iFont != m_vecFonts.end(); ++iFont)
  {
    if ((*iFont)->GetFontName() == strFontName)
    {
      delete (*iFont);
      m_vecFonts.erase(iFont);
      return;
    }
  }
}

void GUIFontManager::FreeFontFile(CGUIFontTTFBase *pFont)
{
  for (vector<CGUIFontTTFBase*>::iterator it = m_vecFontFiles.begin(); it != m_vecFontFiles.end(); ++it)
  {
    if (pFont == *it)
    {
      m_vecFontFiles.erase(it);
      delete pFont;
      return;
    }
  }
}

CGUIFontTTFBase* GUIFontManager::GetFontFile(const CStdString& strFileName)
{
  for (int i = 0; i < (int)m_vecFontFiles.size(); ++i)
  {
    CGUIFontTTFBase* pFont = (CGUIFontTTFBase *)m_vecFontFiles[i];
    if (pFont->GetFileName() == strFileName)
      return pFont;
  }
  return NULL;
}

CGUIFont* GUIFontManager::GetFont(const CStdString& strFontName, bool fallback /*= true*/)
{
  for (int i = 0; i < (int)m_vecFonts.size(); ++i)
  {
    CGUIFont* pFont = m_vecFonts[i];
    if (pFont->GetFontName() == strFontName)
      return pFont;
  }
  // fall back to "font16" if we have none
  if (fallback && !strFontName.IsEmpty() && !strFontName.Equals("-") && !strFontName.Equals("font16"))
    return GetFont("font16", false);
  return NULL;
}

void GUIFontManager::Clear()
{
  for (int i = 0; i < (int)m_vecFonts.size(); ++i)
  {
    CGUIFont* pFont = m_vecFonts[i];
    delete pFont;
  }

  m_vecFonts.clear();
  m_vecFontFiles.clear();
  m_vecFontInfo.clear();
  m_fontsetUnicode=false;
}

void GUIFontManager::LoadFonts(const CStdString& strFontSet)
{
  TiXmlDocument xmlDoc;
  if (!OpenFontFile(xmlDoc))
    return;

  TiXmlElement* pRootElement = xmlDoc.RootElement();
  const TiXmlNode *pChild = pRootElement->FirstChild();

  // If there are no fontset's defined in the XML (old skin format) run in backward compatibility
  // and ignore the fontset request
  CStdString strValue = pChild->Value();
  if (strValue == "fontset")
  {
    CStdString foundTTF;
    while (pChild)
    {
      strValue = pChild->Value();
      if (strValue == "fontset")
      {
        const char* idAttr = ((TiXmlElement*) pChild)->Attribute("id");

        const char* unicodeAttr = ((TiXmlElement*) pChild)->Attribute("unicode");

        if (foundTTF.IsEmpty() && idAttr != NULL && unicodeAttr != NULL && stricmp(unicodeAttr, "true") == 0)
          foundTTF = idAttr;

        // Check if this is the fontset that we want
        if (idAttr != NULL && stricmp(strFontSet.c_str(), idAttr) == 0)
        {
          m_fontsetUnicode=false;
          // Check if this is the a ttf fontset
          if (unicodeAttr != NULL && stricmp(unicodeAttr, "true") == 0)
            m_fontsetUnicode=true;

          if (m_fontsetUnicode)
          {
            LoadFonts(pChild->FirstChild());
            break;
          }
        }

      }

      pChild = pChild->NextSibling();
    }

    // If no fontset was loaded
    if (pChild == NULL)
    {
      CLog::Log(LOGWARNING, "file doesnt have <fontset> with name '%s', defaulting to first fontset", strFontSet.c_str());
      if (!foundTTF.IsEmpty())
        LoadFonts(foundTTF);
    }
  }
  else
  {
    CLog::Log(LOGERROR, "file doesnt have <fontset> in <fonts>, but rather %s", strValue.c_str());
    return ;
  }
}

void GUIFontManager::LoadFonts(const TiXmlNode* fontNode)
{
  while (fontNode)
  {
    CStdString strValue = fontNode->Value();
    if (strValue == "font")
    {
      const TiXmlNode *pNode = fontNode->FirstChild("name");
      if (pNode)
      {
        CStdString strFontName = pNode->FirstChild()->Value();
        color_t shadowColor = 0;
        color_t textColor = 0;
        CGUIControlFactory::GetColor(fontNode, "shadow", shadowColor);
        CGUIControlFactory::GetColor(fontNode, "color", textColor);
        const TiXmlNode *pNode = fontNode->FirstChild("filename");
        if (pNode)
        {
          CStdString strFontFileName = pNode->FirstChild()->Value();
          if (strFontFileName.Find(".ttf") >= 0)
          {
            int iSize = 20;
            int iStyle = FONT_STYLE_NORMAL;
            float aspect = 1.0f;
            float lineSpacing = 1.0f;

            XMLUtils::GetInt(fontNode, "size", iSize);
            if (iSize <= 0) iSize = 20;

            pNode = fontNode->FirstChild("style");
            if (pNode)
            {
              CStdString style = pNode->FirstChild()->Value();
              iStyle = FONT_STYLE_NORMAL;
              if (style == "bold")
                iStyle = FONT_STYLE_BOLD;
              else if (style == "italics")
                iStyle = FONT_STYLE_ITALICS;
              else if (style == "bolditalics")
                iStyle = FONT_STYLE_BOLD_ITALICS;
            }

            XMLUtils::GetFloat(fontNode, "linespacing", lineSpacing);
            XMLUtils::GetFloat(fontNode, "aspect", aspect);

            LoadTTF(strFontName, strFontFileName, textColor, shadowColor, iSize, iStyle, lineSpacing, aspect);
          }
        }
      }
    }

    fontNode = fontNode->NextSibling();
  }
}

bool GUIFontManager::OpenFontFile(TiXmlDocument& xmlDoc)
{
  // Get the file to load fonts from:
  CStdString strPath = g_SkinInfo.GetSkinPath("Font.xml", &m_skinResolution);
  CLog::Log(LOGINFO, "Loading fonts from %s", strPath.c_str());

  // first try our preferred file
  if ( !xmlDoc.LoadFile(strPath) )
  {
    CLog::Log(LOGERROR, "Couldn't load %s", strPath.c_str());
    return false;
  }
  TiXmlElement* pRootElement = xmlDoc.RootElement();

  CStdString strValue = pRootElement->Value();
  if (strValue != CStdString("fonts"))
  {
    CLog::Log(LOGERROR, "file %s doesnt start with <fonts>", strPath.c_str());
    return false;
  }

  return true;
}

bool GUIFontManager::GetFirstFontSetUnicode(CStdString& strFontSet)
{
  strFontSet.Empty();

  // Load our font file
  TiXmlDocument xmlDoc;
  if (!OpenFontFile(xmlDoc))
    return false;

  TiXmlElement* pRootElement = xmlDoc.RootElement();
  const TiXmlNode *pChild = pRootElement->FirstChild();

  CStdString strValue = pChild->Value();
  if (strValue == "fontset")
  {
    while (pChild)
    {
      strValue = pChild->Value();
      if (strValue == "fontset")
      {
        const char* idAttr = ((TiXmlElement*) pChild)->Attribute("id");

        const char* unicodeAttr = ((TiXmlElement*) pChild)->Attribute("unicode");

        // Check if this is a fontset with a ttf attribute set to true
        if (unicodeAttr != NULL && stricmp(unicodeAttr, "true") == 0)
        {
          //  This is the first ttf fontset
          strFontSet=idAttr;
          break;
        }

      }

      pChild = pChild->NextSibling();
    }

    // If no fontset was loaded
    if (pChild == NULL)
      CLog::Log(LOGWARNING, "file doesnt have <fontset> with with attibute unicode=\"true\"");
  }
  else
  {
    CLog::Log(LOGERROR, "file doesnt have <fontset> in <fonts>, but rather %s", strValue.c_str());
  }

  return !strFontSet.IsEmpty();
}

bool GUIFontManager::IsFontSetUnicode(const CStdString& strFontSet)
{
  TiXmlDocument xmlDoc;
  if (!OpenFontFile(xmlDoc))
    return false;

  TiXmlElement* pRootElement = xmlDoc.RootElement();
  const TiXmlNode *pChild = pRootElement->FirstChild();

  CStdString strValue = pChild->Value();
  if (strValue == "fontset")
  {
    while (pChild)
    {
      strValue = pChild->Value();
      if (strValue == "fontset")
      {
        const char* idAttr = ((TiXmlElement*) pChild)->Attribute("id");

        const char* unicodeAttr = ((TiXmlElement*) pChild)->Attribute("unicode");

        // Check if this is the fontset that we want
        if (idAttr != NULL && stricmp(strFontSet.c_str(), idAttr) == 0)
          return (unicodeAttr != NULL && stricmp(unicodeAttr, "true") == 0);

      }

      pChild = pChild->NextSibling();
    }
  }

  return false;
}
