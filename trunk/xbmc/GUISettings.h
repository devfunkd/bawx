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
#include <map>
#include "Resolution.h"
#include "InfoPage.h"

// defined as macro in Windowsx.h (in windows SDK) - and confuses compiler since CSettingsGroup has same name method
#undef GetWindowID

class TiXmlNode;
class TiXmlElement;

// Render Methods
#define RENDER_METHOD_AUTO      0
#define RENDER_METHOD_ARB       1
#define RENDER_METHOD_GLSL      2
#define RENDER_METHOD_SOFTWARE  3
#define RENDER_METHOD_D3D_PS    4
#define RENDER_OVERLAYS         99   // to retain compatibility


// Scaling options.
#define SOFTWARE_UPSCALING_DISABLED   0
#define SOFTWARE_UPSCALING_SD_CONTENT 1
#define SOFTWARE_UPSCALING_ALWAYS     2

// Display blanking options.
#define BLANKING_DISABLED     0
#define BLANKING_ALL_DISPLAYS 1

#ifdef __APPLE__

// Apple Remote options.
#define APPLE_REMOTE_DISABLED  0
#define APPLE_REMOTE_STANDARD  1
#define APPLE_REMOTE_UNIVERSAL 2 
#define APPLE_REMOTE_MULTIREMOTE  3

#endif

// Subtitle colours

#define SUBTITLE_COLOR_START  0
#define SUBTITLE_COLOR_END    5

// Karaoke colours

// If you want to add more colors, it should be done the following way:
// 1. Increase KARAOKE_COLOR_END
// 2. Add a new color description in language/English/strings.xml in block
//    with id 22040 + KARAOKE_COLOR_END value
// 3. Add a new color hex mask into gLyricColors structure in karaoke/karaokelyricstext.cpp
#define KARAOKE_COLOR_START  0
#define KARAOKE_COLOR_END    4

// CDDA ripper defines
#define CDDARIP_ENCODER_LAME     0
#define CDDARIP_ENCODER_VORBIS   1
#define CDDARIP_ENCODER_WAV      2

#define CDDARIP_QUALITY_CBR      0
#define CDDARIP_QUALITY_MEDIUM   1
#define CDDARIP_QUALITY_STANDARD 2
#define CDDARIP_QUALITY_EXTREME  3

#define AUDIO_ANALOG         0
#define AUDIO_DIGITAL_SPDIF  1
#define AUDIO_DIGITAL_HDMI   2
#define DD_TRUEHD_DRC_AUTO       0
#define DD_TRUEHD_DRC_OFF        1
#define DD_TRUEHD_DRC_ON         2
#define STAGING_ON           0
#define DD_TRUEHD_DRC_PRC_MIN    0
#define DD_TRUEHD_DRC_PRC_MAX    100
#ifdef HAS_INTEL_SMD
#define AUDIO_ALL_OUTPUTS    3
#endif

#define AUDIO_LIBRARY_ALSA       0
#define AUDIO_LIBRARY_PULSEAUDIO 1

#define VIDEO_NORMAL 0
#define VIDEO_LETTERBOX 1
#define VIDEO_WIDESCREEN 2

// LCD settings
#define LCD_TYPE_NONE        0
#define LCD_TYPE_LCD_HD44780 1
#define LCD_TYPE_LCD_KS0073  2
#define LCD_TYPE_VFD         3
#define LCD_TYPE_LCDPROC     4

#define MODCHIP_SMARTXX   0
#define MODCHIP_XENIUM    1
#define MODCHIP_XECUTER3  2

// LED settings
#define LED_COLOUR_NO_CHANGE 0
#define LED_COLOUR_GREEN   1
#define LED_COLOUR_ORANGE   2
#define LED_COLOUR_RED    3
#define LED_COLOUR_CYCLE   4
#define LED_COLOUR_OFF    5

#define FRAME_RATE_LEAVE_AS_IS  0
#define FRAME_RATE_CONVERT      1
#define FRAME_RATE_USE_PAL60    2

#define LED_PLAYBACK_OFF     0
#define LED_PLAYBACK_VIDEO    1
#define LED_PLAYBACK_MUSIC    2
#define LED_PLAYBACK_VIDEO_MUSIC 3

#define SPIN_DOWN_NONE  0
#define SPIN_DOWN_MUSIC  1
#define SPIN_DOWN_VIDEO  2
#define SPIN_DOWN_BOTH  3

#define AAM_QUIET 1
#define AAM_FAST  0

#define APM_HIPOWER 0
#define APM_LOPOWER 1
#define APM_HIPOWER_STANDBY 2
#define APM_LOPOWER_STANDBY 3

#define SETTINGS_TYPE_BOOL   1
#define SETTINGS_TYPE_FLOAT   2
#define SETTINGS_TYPE_INT    3
#define SETTINGS_TYPE_STRING  4
#define SETTINGS_TYPE_HEX    5
#define SETTINGS_TYPE_SEPARATOR 6
#define SETTINGS_TYPE_PATH      7

#define CHECKMARK_CONTROL           1
#define SPIN_CONTROL_FLOAT          2
#define SPIN_CONTROL_INT            3
#define SPIN_CONTROL_INT_PLUS       4
#define SPIN_CONTROL_TEXT           5
#define EDIT_CONTROL_INPUT          6
#define EDIT_CONTROL_HIDDEN_INPUT   7
#define EDIT_CONTROL_NUMBER_INPUT   8
#define EDIT_CONTROL_IP_INPUT       9
#define BUTTON_CONTROL_STANDARD    10
#define BUTTON_CONTROL_MISC_INPUT  11
#define BUTTON_CONTROL_PATH_INPUT  12
#define SEPARATOR_CONTROL          13

#define RESUME_NO  0
#define RESUME_YES 1
#define RESUME_ASK 2

#define REPLAY_GAIN_NONE 0
#define REPLAY_GAIN_ALBUM 1
#define REPLAY_GAIN_TRACK 2

//AV sync options
#define SYNC_DISCON 0
#define SYNC_SKIPDUP 1
#define SYNC_RESAMPLE 2

//resampler quality
#define RESAMPLE_LOW 0
#define RESAMPLE_MID 1
#define RESAMPLE_HIGH 2
#define RESAMPLE_REALLYHIGH 3

//Netflix / Play/Pause button
#define NRDPP_DEFAULT   0
#define NRDPP_NETFLIX   1
#define NRDPP_PLAYPAUSE 2

enum PowerState
{
  POWERSTATE_QUIT      = 0,
  POWERSTATE_SHUTDOWN,
  POWERSTATE_HIBERNATE,
  POWERSTATE_SUSPEND,
  POWERSTATE_REBOOT,
  POWERSTATE_MINIMIZE,
  POWERSTATE_NONE,
  POWERSTATE_ASK
};

// replay gain settings struct for quick access by the player multiple
// times per second (saves doing settings lookup)
struct ReplayGainSettings
{
  int iPreAmp;
  int iNoGainPreAmp;
  int iType;
  bool bAvoidClipping;
};

// base class for all settings types
class CSetting
{
public:
  CSetting(int iOrder, const char *strSetting, int iLabel, int iControlType, bool useILabel = true, CStdString labelStr = "") {
    m_iOrder = iOrder;
    m_strSetting = strSetting;
    m_iLabel = iLabel;
    m_iControlType = iControlType;
    m_advanced = false;
    m_visible = true;
    m_useILabel = useILabel;
    m_labelStr = labelStr;

  };
  virtual ~CSetting() {};
  virtual int GetType() { return 0; };
  int GetControlType() { return m_iControlType; };
  virtual void FromString(const CStdString &strValue) {};
  virtual CStdString ToString() { return ""; };
  const char *GetSetting() { return m_strSetting.c_str(); };
  int GetLabel() { return m_iLabel; };
  CStdString GetLabelStr();
  int GetOrder() const { return m_iOrder; };
  void SetAdvanced() { m_advanced = true; };
  bool IsAdvanced() { return m_advanced; };
  // A setting might be invisible in the current session, yet carried over
  // in the config file.
  void SetVisible(bool visible) { m_visible = visible; }
  bool IsVisible() { return m_visible; }
  void SetCustomData(CStdString customData) { m_customData = customData;}
  CStdString GetCustomData() { return m_customData; }
private:
  int m_iControlType;
  int m_iLabel;
  int m_iOrder;
  bool m_advanced;
  bool m_visible;
  CStdString m_strSetting;
  bool m_useILabel;
  CStdString m_labelStr;
  CStdString m_customData;
};

class CSettingBool : public CSetting
{
public:
  CSettingBool(int iOrder, const char *strSetting, int iLabel, bool bData, int iControlType): CSetting(iOrder, strSetting, iLabel, iControlType) { m_bData = bData; };
  CSettingBool(int iOrder, const char *strSetting, CStdString labelStr, bool bData, int iControlType): CSetting(iOrder, strSetting, 0, iControlType, false, labelStr) { m_bData = bData; };
  virtual ~CSettingBool() {};

  virtual int GetType() { return SETTINGS_TYPE_BOOL; };
  virtual void FromString(const CStdString &strValue);
  virtual CStdString ToString();

  void SetData(bool bData) { m_bData = bData; };
  bool GetData() const { return m_bData; };

private:
  bool m_bData;
};

class CSettingFloat : public CSetting
{
public:
  CSettingFloat(int iOrder, const char *strSetting, int iLabel, float fData, float fMin, float fStep, float fMax, int iControlType);
  virtual ~CSettingFloat() {};

  virtual int GetType() { return SETTINGS_TYPE_FLOAT; };
  virtual void FromString(const CStdString &strValue);
  virtual CStdString ToString();

  void SetData(float fData) { m_fData = fData; if (m_fData < m_fMin) m_fData = m_fMin; if (m_fData > m_fMax) m_fData = m_fMax;};
float GetData() const { return m_fData; };

  float m_fMin;
  float m_fStep;
  float m_fMax;

private:
  float m_fData;
};

class CSettingInt : public CSetting
{
public:
  CSettingInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, const char *strFormat);
  CSettingInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, int iFormat, int iLabelMin);
  virtual ~CSettingInt() {};

  virtual int GetType() { return SETTINGS_TYPE_INT; };
  virtual void FromString(const CStdString &strValue);
  virtual CStdString ToString();

  void SetData(int iData) { m_iData = iData; if (m_iData < m_iMin) m_iData = m_iMin; if (m_iData > m_iMax) m_iData = m_iMax;};
int GetData() const { return m_iData; };

  int m_iMin;
  int m_iStep;
  int m_iMax;
  int m_iFormat;
  int m_iLabelMin;
  CStdString m_strFormat;

protected:
  int m_iData;
};

class CSettingHex : public CSettingInt
{
public:
  CSettingHex(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, const char *strFormat)
      : CSettingInt(iOrder, strSetting, iLabel, iData, iMin, iStep, iMax, iControlType, strFormat) {};
  virtual ~CSettingHex() {};
  virtual void FromString(const CStdString &strValue);
  virtual CStdString ToString();
  virtual int GetType() { return SETTINGS_TYPE_HEX; };
};

class CSettingString : public CSetting
{
public:
  CSettingString(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType, bool bAllowEmpty, int iHeadingString);
	  CSettingString(int iOrder, const char *strSetting, CStdString labelStr, const char *strData, int iControlType, bool bAllowEmpty, int iHeadingString);
  virtual ~CSettingString() {};

  virtual int GetType() { return SETTINGS_TYPE_STRING; };
  virtual void FromString(const CStdString &strValue);
  virtual CStdString ToString();

  void SetData(const char *strData) { m_strData = strData; };
  const CStdString &GetData() const { return m_strData; };

  bool m_bAllowEmpty;
  int m_iHeadingString;
private:
  CStdString m_strData;
};

class CSettingPath : public CSettingString
{
public:
  CSettingPath(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType, bool bAllowEmpty, int iHeadingString);
  virtual ~CSettingPath() {};

  virtual int GetType() { return SETTINGS_TYPE_PATH; };
};

class CSettingSeparator : public CSetting
{
public:
  CSettingSeparator(int iOrder, const char *strSetting);
  virtual ~CSettingSeparator() {};

  virtual int GetType() { return SETTINGS_TYPE_SEPARATOR; };
};

typedef std::vector<CSetting *> vecSettings;

class CSettingsCategory
{
public:
  CSettingsCategory(const char *strCategory, int labelID, bool runTime = false)
  {
    m_strCategory = strCategory;
    m_labelID = labelID;
    m_runTime = runTime;
  }
  ~CSettingsCategory()
  {
    m_vecSettings.clear(); 
  }

  void AddSetting(CSetting* setting)
  {
    m_vecSettings.push_back(setting);
  }
  
  CStdString m_strCategory;
  int m_labelID;
  bool m_runTime;
  vecSettings m_vecSettings;
};

typedef std::vector<CSettingsCategory *> vecSettingsCategory;

class CSettingsGroup
{
public:
  CSettingsGroup(int groupID, int labelID, int windowID = 0, int titleID = 0)
  {
    m_groupID = groupID;
    m_labelID = labelID;
    m_windowID = windowID;
    m_titleID = titleID;
  }
  ~CSettingsGroup()
  {
    for (unsigned int i = 0; i < m_vecCategories.size(); i++)
      delete m_vecCategories[i];
    m_vecCategories.clear();
  };

  CSettingsCategory* AddCategory(const char *strCategory, int labelID, bool runtime = false)
  {
    CSettingsCategory *pCategory = new CSettingsCategory(strCategory, labelID, runtime);
    if (!pCategory)
    {
      return NULL;
    }
    
    m_vecCategories.push_back(pCategory);
    return pCategory;
  }
  
  void GetCategories(vecSettingsCategory &vecCategories);
  int GetLabelID() { return m_labelID; }
  int GetGroupID() { return m_groupID; }
  int GetWindowID() { return m_windowID; }
  int GetTitleID() { return m_titleID; }
private:
  vecSettingsCategory m_vecCategories;
  int m_groupID;
  int m_labelID;
  int m_windowID;
  int m_titleID;
};

class CGUISettings : public InfoPageble
{
public:
  CGUISettings();
  virtual ~CGUISettings();

  void Initialize();
  virtual void GetInfoPage(CStdString *str, CStdString params);
  void AddGroup(int groupID, int labelID);
  CSettingsCategory* AddCategory(int groupID, const char *strCategory, int labelID);
  CSettingsGroup *GetGroup(int windowID);
  CSettingsGroup *GetGroupByWindowId(int windowId);

  void AddBool(int iOrder, const char *strSetting, int iLabel, bool bSetting, int iControlType = CHECKMARK_CONTROL);
  void AddBool(int iOrder, const char *strSetting, CStdString strLabel, bool bSetting, int iControlType = CHECKMARK_CONTROL);
  bool GetBool(const char *strSetting) const;
  void SetBool(const char *strSetting, bool bSetting);
  void ToggleBool(const char *strSetting);

  void AddFloat(int iOrder, const char *strSetting, int iLabel, float fSetting, float fMin, float fStep, float fMax, int iControlType = SPIN_CONTROL_FLOAT);
  float GetFloat(const char *strSetting) const;
  void SetFloat(const char *strSetting, float fSetting);

  void AddInt(int iOrder, const char *strSetting, int iLabel, int fSetting, int iMin, int iStep, int iMax, int iControlType, const char *strFormat = NULL);
  void AddInt(int iOrder, const char *strSetting, int iLabel, int iData, int iMin, int iStep, int iMax, int iControlType, int iFormat, int iLabelMin=-1);
  int GetInt(const char *strSetting) const;
  void SetInt(const char *strSetting, int fSetting);

  void AddHex(int iOrder, const char *strSetting, int iLabel, int fSetting, int iMin, int iStep, int iMax, int iControlType, const char *strFormat = NULL);

  void AddString(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType = EDIT_CONTROL_INPUT, bool bAllowEmpty = false, int iHeadingString = -1);
  void AddString(int iOrder, const char *strSetting,  CStdString strLabel, const char *strData, int iControlType = EDIT_CONTROL_INPUT, bool bAllowEmpty = false, int iHeadingString = -1);
  void AddPath(int iOrder, const char *strSetting, int iLabel, const char *strData, int iControlType = EDIT_CONTROL_INPUT, bool bAllowEmpty = false, int iHeadingString = -1);

  const CStdString &GetString(const char *strSetting, bool bPrompt=true) const;
  void SetString(const char *strSetting, const char *strData);

  void AddSeparator(int iOrder, const char *strSetting);

  CSetting *GetSetting(const char *strSetting);
  bool HasSetting(const char *strSetting);
  void     DeleteSetting(const char *strSetting);

  void GetSettingsGroup(const char *strGroup, vecSettings &settings);
  void LoadXML(TiXmlElement *pRootElement, bool hideSettings = false);
  void SaveXML(TiXmlNode *pRootNode);
  void LoadMasterLock(TiXmlElement *pRootElement);

  RESOLUTION GetResolution() const;
  static RESOLUTION GetResFromString(const CStdString &res);
  void SetResolution(RESOLUTION res);

  //m_LookAndFeelResolution holds the real gui resolution
  RESOLUTION m_LookAndFeelResolution;
  ReplayGainSettings m_replayGain;

  void Clear();

  bool IsUsingCustomSettingsOrder() { return m_customSettingsOrder; }
  void LoadCustomSettingsOrder();
  
private:
  
  typedef std::map<CStdString, CSetting*>::iterator mapIter;
  typedef std::map<CStdString, CSetting*>::const_iterator constMapIter;
  std::map<CStdString, CSetting*> settingsMap;
  std::vector<CSettingsGroup *> settingsGroups;
  void LoadFromXML(TiXmlElement *pRootElement, mapIter &it, bool advanced = false);
  
  bool m_customSettingsOrder;
};

extern class CGUISettings g_guiSettings;
