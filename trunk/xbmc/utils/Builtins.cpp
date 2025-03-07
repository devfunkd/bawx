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
#include "AlarmClock.h"
#include "Application.h"
#include "Autorun.h"
#include "Builtins.h"
#include "ButtonTranslator.h"
#include "FileItem.h"
#include "GUIDialogFileBrowser.h"
#include "GUIDialogKeyboard.h"
#include "GUIDialogMusicScan.h"
#include "GUIDialogNumeric.h"
#include "GUIDialogVideoScan.h"
#include "GUIUserMessages.h"
#include "GUIWindowVideoBase.h"
#ifdef HAS_LASTFM
#include "LastFmManager.h"
#endif
#include "LCD.h"
#include "log.h"
#include "MediaManager.h"
#include "RssReader.h"
#ifndef _BOXEE_
#include "PartyModeManager.h"
#endif
#include "Settings.h"
#include "StringUtils.h"
#include "Util.h"

#include "FileSystem/PluginDirectory.h"
#include "FileSystem/RarManager.h"
#include "FileSystem/ZipManager.h"

#include "GUIDialogBoxeeMediaAction.h"
#include "GUIDialogBoxeeVideoCtx.h"
#include "GUIDialogBoxeeMusicCtx.h"
#include "GUIDialogBoxeePictureCtx.h"

#include "GUIWindowManager.h"
#include "LocalizeStrings.h"
#include "system.h"
#ifdef _WIN32
#include "GUISettings.h"
#endif

#ifdef HAS_LIRC
#include "common/LIRC.h"
#endif
#ifdef HAS_IRSERVERSUITE
  #include "common/IRServerSuite/IRServerSuite.h"
#endif

#ifdef HAS_PYTHON
#include "lib/libPython/XBPython.h"
#endif

#ifdef HAS_WEB_SERVER
#include "lib/libGoAhead/XBMChttp.h"
#include "lib/libGoAhead/WebServer.h"
#endif

#include "AppManager.h"
#include "GUIWindowApp.h"
#include "GUIDialogYesNo.h"
#include "GUIDialogYesNo2.h"
#include "GUIToggleButtonControl.h"
#include "GUIInfoManager.h"

#ifdef _WIN32
#include "RemoteWrapper.h"
#endif

#include <vector>

using namespace std;
using namespace DIRECTORY;
#ifdef HAS_DVD_DRIVE
using namespace MEDIA_DETECT;
#endif

typedef struct
{
  const char* command;
  bool needsParameters;
  const char* description;
} BUILT_IN;

const BUILT_IN commands[] = {
  { "Help",                       false,  "This help message" },
  { "Reboot",                     false,  "Reboot the xbox (power cycle)" },
  { "Restart",                    false,  "Restart the xbox (power cycle)" },
  { "ShutDown",                   false,  "Shutdown the xbox" },
  { "Powerdown",                  false,  "Powerdown system" },
  { "Quit",                       false,  "Quit XBMC" },
  { "Hibernate",                  false,  "Hibernates the system" },
  { "Suspend",                    false,  "Suspends the system" },
  { "logout",                     false,  "logout user from XBMC" },
  { "RestartApp",                 false,  "Restart XBMC" },
  { "Minimize",                   false,  "Minimize XBMC" },
  { "Credits",                    false,  "Run XBMCs Credits" },
  { "Reset",                      false,  "Reset the xbox (warm reboot)" },
  { "Mastermode",                 false,  "Control master mode" },
  { "ActivateWindow",             true,   "Activate the specified window" },
  { "ReplaceWindow",              true,   "Replaces the current window with the new one" },
  { "TakeScreenshot",             false,  "Takes a Screenshot" },
  { "RunScript",                  true,   "Run the specified script" },
  { "RunPlugin",                  true,   "Run the specified plugin" },
  { "RunApp",                     true,   "Run the specified app" },
  { "CallPython",                 true,   "Run the specified python code" },
  { "ShowActionMenu",             true,   "Show action menu of selected file item in list" },
  { "Extract",                    true,   "Extracts the specified archive" },
  { "PlayMedia",                  true,   "Play the specified media file (or playlist)" },
  { "SlideShow",                  true,   "Run a slideshow from the specified directory" },
  { "RecursiveSlideShow",         true,   "Run a slideshow from the specified directory, including all subdirs" },
  { "ReloadSkin",                 false,  "Reload XBMC's skin" },
  { "RefreshRSS",                 false,  "Reload RSS feeds from RSSFeeds.xml"},
  { "PlayerControl",              true,   "Control the music or video player" },
  { "Playlist.PlayOffset",        true,   "Start playing from a particular offset in the playlist" },
  { "Playlist.Clear",             false,  "Clear the current playlist" },
  { "EjectTray",                  false,  "Close or open the DVD tray" },
  { "AlarmClock",                 true,   "Prompt for a length of time and start an alarm clock" },
  { "CancelAlarm",                true,   "Cancels an alarm" },
  { "Action",                     true,   "Executes an action for the active window (same as in keymap)" },
  { "Notification",               true,   "Shows a notification on screen, specify header, then message, and optionally time in milliseconds and a icon." },
  { "PlayDVD",                    false,  "Plays the inserted CD or DVD media from the DVD-ROM Drive!" },
  { "Skin.ToggleSetting",         true,   "Toggles a skin setting on or off" },
  { "Skin.SetString",             true,   "Prompts and sets skin string" },
  { "Skin.SetNumeric",            true,   "Prompts and sets numeric input" },
  { "Skin.SetPath",               true,   "Prompts and sets a skin path" },
  { "Skin.Theme",                 true,   "Control skin theme" },
  { "Skin.SetImage",              true,   "Prompts and sets a skin image" },
  { "Skin.SetLargeImage",         true,   "Prompts and sets a large skin images" },
  { "Skin.SetFile",               true,   "Prompts and sets a file" },
  { "Skin.SetBool",               true,   "Sets a skin setting on" },
  { "Skin.Reset",                 true,   "Resets a skin setting to default" },
  { "Skin.ResetSettings",         false,  "Resets all sMute the playerkin settings" },
  { "Mute",                       false,  "Mute the player" },
  { "SetVolume",                  true,   "Set the current volume" },
  { "VolumeUp",                   false,  "Volume up" },
  { "VolumeDown",                 false,  "Volume down" },
  { "Dialog.Close",               true,   "Close a dialog" },
  { "System.LogOff",              false,  "Log off current user" },
  { "System.Exec",                true,   "Execute shell commands" },
  { "System.ExecWait",            true,   "Execute shell commands and freezes XBMC until shell is closed" },
  { "Resolution",                 true,   "Change XBMC's Resolution" },
  { "SetFocus",                   true,   "Change current focus to a different control id" },
  { "SetVisible",                 true,   "set a control to be visible" },
  { "SetHidden",                  true,   "set a control to be hidden" },
  { "BackupSystemInfo",           false,  "Backup System Informations to local hdd" },
  { "UpdateLibrary",              true,   "Update the selected library (music or video)" },
  { "CleanLibrary",               true,   "Clean the video library" },
  { "PageDown",                   true,   "Send a page down event to the pagecontrol with given id" },
  { "PageUp",                     true,   "Send a page up event to the pagecontrol with given id" },
#ifdef HAS_LASTFM
  { "LastFM.Love",                false,  "Add the current playing last.fm radio track to the last.fm loved tracks" },
  { "LastFM.Ban",                 false,  "Ban the current playing last.fm radio track" },
  { "LastFM.Settings",            false,  "show the lastfm settings dialog" },
#endif
  { "Container.Refresh",          false,  "Refresh current listing" },
  { "Container.Update",           false,  "Update current listing. Send Container.Update(path,replace) to reset the path history" },
  { "Container.NextViewMode",     false,  "Move to the next view type (and refresh the listing)" },
  { "Container.PreviousViewMode", false,  "Move to the previous view type (and refresh the listing)" },
  { "Container.SetViewMode",      true,   "Move to the view with the given id" },
  { "System.Exec",      	        true,   "Execute shell commands" },
  { "Container.NextSortMethod",   false,  "Change to the next sort method" },
  { "Container.PreviousSortMethod",false, "Change to the previous sort method" },
  { "Container.SetSortMethod",    true,   "Change to the specified sort method" },
  { "Container.SetPath",          true,   "Set new path for container" },
  { "Container.SortDirection",    false,  "Toggle the sort direction" },
  { "Container.FocusItem",        true,   "Focus item in a container" },
  { "Container.SelectFocusedItem", true,   "Select the focused item in a container" },
  { "Container.SelectItem",       true,   "Select single item in a container" },
  { "Container.UnselectItem",     true,   "Unselect single item in a container" },
  { "Container.SelectAll",        true,   "Select all items in a container" },
  { "Container.UnselectAll",      true,   "Unselect all items in a container" },
  { "Control.Move",               true,   "Tells the specified control to 'move' to another entry specified by offset" },
  { "Control.SetFocus",           true,   "Change current focus to a different control id" },
  { "Control.Message",            true,   "Send a given message to a control within a given window" },
  { "Window.SetProperty",         true,   "Set specified value to a window property" },
  { "Window.Refresh",             false,   "Send update message to current window" },
  { "ResetDatabase",              false,  "Reset library database" },
  { "CleanOldThumbnails",         false,  "Clean old thumbnails" },
  { "SaveWindowState",            false,  "Saves the state of the current active window" },
  { "ResetWindowState",           false,  "Reset all previous saved states of the current active window and restore to topmost state" },
  { "RestoreWindowState",         false,  "Restore the previously saved saved states of the current active window" },
  { "PopWindowState",             false,  "Pop the previously saved states of the current active window without restoring the state" },
  { "PopToWindowState",           true,   "Pop previously saved states of the current active window without restoring the state, keeping N items in the stack" },
  { "SendClick",                  true,   "Send a click message from the given control to the given window" },
  { "Control.SetLabel",           true,   "Set a label value of a control" },       // SetLabel(id, string)
  { "ToggleButton.Select",        true,  "Select a toggle button" },
  { "ToggleButton.Unselect",      true,  "Unselect a toggle button" },
  { "App.SetString",              true,   "Prompts and sets app string" },          // App.SetString(key, value[, keyboard title[, hidden]])
  { "App.SetBool",                true,   "Sets a skin setting on" },               // App.SetBool(key, true|false)
  { "App.Toggle",                 true,   "Toggles a setting on or off" },          // App.Toggle(key) if doesn't exist, die
  { "App.ResetAll",               false,  "Deletes all keys" },                     // App.ResetAll()
  { "App.Reset",                  true,   "Deletes a key" },                        // App.Reset(key)
  { "App.PushBackString",         true,   "Prompts and set string" },               // App.PushBackString(key, value, limit (0=none)[, keyboard title])
  { "App.PushFrontString",        true,   "Prompts and set string" },               // App.PushFrontString(key, value, limit (0=none)[, keyboard title])
  { "App.SetParam",               true,   "Set parameter value" },
  { "Log.Debug",                  true,   "Log at debug level" },  
  { "Log.Error",                  true,   "Log at error level" },  
  { "Log.Info",                   true,   "Log at info level" },  
  { "Log.Warning",                true,   "Log at warning level" },
  { "LoadProfile",                true,   "Load the specified profile (note; if locks are active it won't work)" },
  { "SetProperty",                true,   "Sets a window property for the current window (key,value)" },
  { "PlayWith",                   true,   "Play the selected item with the specified core" },
#if defined(HAS_LIRC) || defined(HAS_IRSERVERSUITE)
  { "LIRC.Stop",                  false,  "Removes XBMC as LIRC client" },
  { "LIRC.Start",                 false,  "Adds XBMC as LIRC client" },
#endif
#ifdef HAS_LCD
  { "LCD.Suspend",                false,  "Suspends LCDproc" },
  { "LCD.Resume",                 false,  "Resumes LCDproc" },
#endif
  { "ToggleKeyboard",             false,  "Toggle keyboard" },
  { "BrowserFullScreen",          true,   "Toggle browser full screen" },
  { "BrowserBack",                false,  "Browser navigation - back" },
  { "BrowserForward",             false,  "Browser navigation - forward" },
  { "BrowserPlaybackPlay",        false,  "Browser playback - play" },
  { "BrowserPlaybackPause",       false,  "Browser playback - pause" },
  { "BrowserPlaybackSkip",        false,  "Browser playback - skip" },
  { "BrowserPlaybackBigSkip",     false,  "Browser playback - big skip" },
  { "BrowserPlaybackBack",        false,  "Browser playback - seek back" },
  { "BrowserPlaybackBigBack",     false,  "Browser playback - big seek back" },
  { "BrowserActivateExt",         true,   "Browser activate extension" },
  { "BrowserPlaybackTogglePause", false,  "Browser playback - pause/play" },
  { "BrowserChangeMode",          true,   "Browser change mode (cursor/keyboard/player)" },
  { "BrowserNavigate",            true,   "Browser navigate (url/search term)" },
  { "BrowserToggleMode",          true,   "Browser toggle between keyboard and mouse mode" },
  { "BrowserToggleQuality",       false,  "Browser toggle between keyboard and mouse mode" },
  { "OpenSearchDialog",           false,  "Open search dialog" },
  { "OsdExtClick",                true,   "Click on extended OSD button" },
};

bool CBuiltins::HasCommand(const CStdString& execString)
{
  CStdString function;
  vector<CStdString> parameters;
  CUtil::SplitExecFunction(execString, function, parameters);
  for (unsigned int i = 0; i < sizeof(commands)/sizeof(BUILT_IN); i++)
  {
    if (function.CompareNoCase(commands[i].command) == 0 && (!commands[i].needsParameters || parameters.size()))
      return true;
  }
  return false;
}

void BrowserToggleQualityFunc(void)
{
  CAction action;
  action.id = ACTION_BROWSER_TOGGLE_QUALITY;
  //action.strAction = params[0];
  if (g_application.m_pPlayer)
    g_application.m_pPlayer->OnAction(action);
}

void CBuiltins::GetHelp(CStdString &help)
{
  help.Empty();
  for (unsigned int i = 0; i < sizeof(commands)/sizeof(BUILT_IN); i++)
  {
    help += commands[i].command;
    help += "\t";
    help += commands[i].description;
    help += "\n";
  }
}

int CBuiltins::Execute(const CStdString& execString)
{
  // Get the text after the "XBMC."
  CStdString execute;
  vector<CStdString> params;
  CUtil::SplitExecFunction(execString, execute, params);
  CStdString parameter = params.size() ? params[0] : "";
  CStdString strParameterCaseIntact = parameter;
  parameter.ToLower();
  execute.ToLower();

  if (execute.Equals("reboot") || execute.Equals("restart"))  //Will reboot the xbox, aka cold reboot
  {
    g_application.getApplicationMessenger().Restart();
  }
  else if (execute.Equals("shutdown"))
  {
    g_application.getApplicationMessenger().Shutdown();
  }
  else if (execute.Equals("logout"))
  {
    g_application.getApplicationMessenger().Logout();
  }
  else if (execute.Equals("powerdown"))
  {
    g_application.getApplicationMessenger().Powerdown();
  }
  else if (execute.Equals("restartapp"))
  {
    g_application.getApplicationMessenger().RestartApp();
  }
  else if (execute.Equals("hibernate"))
  {
    g_application.getApplicationMessenger().Hibernate();
  }
  else if (execute.Equals("suspend"))
  {
    g_application.getApplicationMessenger().Suspend();
  }
  else if (execute.Equals("quit"))
  {
    g_application.getApplicationMessenger().Quit();
  }
  else if (execute.Equals("minimize"))
  {
    g_application.getApplicationMessenger().Minimize();
  }
  else if (execute.Equals("loadprofile") && g_settings.m_vecProfiles[0].getLockMode() == LOCK_MODE_EVERYONE)
  {
    for (unsigned int i=0;i<g_settings.m_vecProfiles.size();++i )
    {
      if (g_settings.m_vecProfiles[i].getName().Equals(parameter))
      {
        g_application.getNetwork().NetworkMessage(CNetwork::SERVICES_DOWN,1);
        g_settings.LoadProfile(i);
        g_application.getNetwork().NetworkMessage(CNetwork::SERVICES_UP,1);
        break;
      }
    }
  }
  else if (execute.Equals("mastermode"))
  {
    if (g_passwordManager.bMasterUser)
    {
      g_passwordManager.bMasterUser = false;
      g_passwordManager.LockSources(true);
      g_application.m_guiDialogKaiToast.QueueNotification(g_localizeStrings.Get(20052),g_localizeStrings.Get(20053));
    }
    else if (g_passwordManager.IsMasterLockUnlocked(true))
    {
      g_passwordManager.LockSources(false);
      g_passwordManager.bMasterUser = true;
      g_application.m_guiDialogKaiToast.QueueNotification(g_localizeStrings.Get(20052),g_localizeStrings.Get(20054));
    }

    CUtil::DeleteVideoDatabaseDirectoryCache();
    CGUIMessage msg(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE);
    g_windowManager.SendMessage(msg);
  }
  else if (execute.Equals("takescreenshot"))
  {
    CUtil::TakeScreenshot();
  }
  else if (execute.Equals("credits"))
  {
#ifdef HAS_CREDITS
    CUtil::RunCredits();
#endif
  }
  else if (execute.Equals("reset")) //Will reset the xbox, aka soft reset
  {
    g_application.getApplicationMessenger().Reset();
  }
  else if (execute.Equals("activatewindow") || execute.Equals("replacewindow"))
  {
    // get the parameters
    CStdString strWindow;
    CStdString strPath;
    if (params.size())
    {
      strWindow = params[0];
      params.erase(params.begin());
    }

    CLog::Log(LOGDEBUG, "ActiveWindow %d (builtin)", g_windowManager.GetActiveWindow());

    // confirm the window destination is valid prior to switching
    int iWindow = CButtonTranslator::TranslateWindowString(strWindow.c_str());
    if (iWindow != WINDOW_INVALID)
    {
      // disable the screensaver
      g_application.WakeUpScreenSaverAndDPMS();
      g_windowManager.ActivateWindow(iWindow, params, !execute.Equals("activatewindow"));
    }
    else
    {
      CLog::Log(LOGERROR, "Activate/ReplaceWindow called with invalid destination window: %s", strWindow.c_str());
      return false;
    }
  }
  else if (execute.Equals("opensearchdialog"))
  {
    std::vector<CStdString> params;
    params.push_back("openinsearch");
    g_windowManager.ActivateWindow(WINDOW_DIALOG_BOXEE_BROWSE_MENU,params);
    return true;
  }
  else if ((execute.Equals("setfocus") || execute.Equals("control.setfocus")) && params.size())
  {
    int controlID = atol(params[0].c_str());
    int subItem = (params.size() > 1) ? atol(params[1].c_str())+1 : 0;

    CGUIMessage msg(GUI_MSG_SETFOCUS, GetRealActiveWindow(), controlID, subItem);
    g_windowManager.SendMessage(msg);
  }
  else if (execute.Equals("setvisible"))
  {
    CGUIMessage msg(GUI_MSG_VISIBLE, GetRealActiveWindow(), atol(parameter.c_str()));
    g_windowManager.SendMessage(msg);
  }
  else if (execute.Equals("sethidden"))
  {
    CGUIMessage msg(GUI_MSG_HIDDEN, GetRealActiveWindow(), atol(parameter.c_str()));
    g_windowManager.SendMessage(msg);
  }
  else if (execute.Equals("showactionmenu"))
  {
    const CGUIControl* pControl = g_windowManager.GetWindow(g_windowManager.GetActiveWindow())->GetControl(atoi(parameter.c_str()));
    if (pControl)
    {
      CFileItem* pSelectedItem = (CFileItem*)((CGUIBaseContainer*)pControl)->GetListItem(0).get();
      if (pSelectedItem)
      {
        CGUIDialogBoxeeMediaAction::ShowAndGetInput(pSelectedItem);
      }
    }
  }
#ifdef HAS_PYTHON
  else if (execute.Equals("callpython"))
  {
    CLog::Log(LOGNOTICE, "Running python script for default application (python)");
    g_pythonParser.evalString(strParameterCaseIntact);
  }
  else if (execute.Equals("runscript") && params.size())
  {
    unsigned int argc = params.size();
    char ** argv = new char*[argc];

    vector<CStdString> path;
    //split the path up to find the filename
    StringUtils::SplitString(params[0],"\\",path);
    argv[0] = path.size() > 0 ? (char*)path[path.size() - 1].c_str() : (char*)params[0].c_str();

    for(unsigned int i = 1; i < argc; i++)
      argv[i] = (char*)params[i].c_str();

    g_pythonParser.evalFile(params[0].c_str(), argc, (const char**)argv);
    delete [] argv;
  }
#endif
  else if (execute.Equals("system.exec"))
  {
    g_application.getApplicationMessenger().Minimize();
    g_application.getApplicationMessenger().ExecOS(parameter, false);
  }
  else if (execute.Equals("system.execwait"))
  {
    g_application.getApplicationMessenger().Minimize();
    g_application.getApplicationMessenger().ExecOS(parameter, true);
  }
  else if (execute.Equals("resolution"))
  {
    RESOLUTION res = RES_PAL_4x3;
    if (parameter.Equals("pal")) res = RES_PAL_4x3;
    else if (parameter.Equals("pal16x9")) res = RES_PAL_16x9;
    else if (parameter.Equals("ntsc")) res = RES_NTSC_4x3;
    else if (parameter.Equals("ntsc16x9")) res = RES_NTSC_16x9;
    else if (parameter.Equals("720p")) res = RES_HDTV_720p;
    else if (parameter.Equals("1080i")) res = RES_HDTV_1080i;
    if (g_graphicsContext.IsValidResolution(res))
    {
      g_guiSettings.SetInt("videoscreen.resolution", res);
      //set the gui resolution, if newRes is RES_AUTORES newRes will be set to the highest available resolution
      g_graphicsContext.SetVideoResolution(res);
      //set our lookandfeelres to the resolution set in graphiccontext
      g_guiSettings.m_LookAndFeelResolution = res;
      g_application.ReloadSkin();
    }
  }
  else if (execute.Equals("extract") && params.size())
  {
    // Detects if file is zip or zip then extracts
    CStdString strDestDirect = "";
    if (params.size() < 2)
      CUtil::GetDirectory(params[0],strDestDirect);
    else
      strDestDirect = params[1];

    CUtil::AddSlashAtEnd(strDestDirect);

    if (CUtil::IsZIP(params[0]))
      g_ZipManager.ExtractArchive(params[0],strDestDirect);
    else if (CUtil::IsRAR(params[0]))
      g_RarManager.ExtractArchive(params[0],strDestDirect);
    else
      CLog::Log(LOGERROR, "XBMC.Extract, No archive given");
  }
  else if (execute.Equals("runplugin"))
  {
    if (params.size())
    {
      CFileItem item(params[0]);
      if (!item.m_bIsFolder)
      {
        item.m_strPath = params[0];
        CPluginDirectory::RunScriptWithParams(item.m_strPath);
      }
    }
    else
    {
      CLog::Log(LOGERROR, "XBMC.RunPlugin called with no arguments.");
    }
  }
  else if (execute.Equals("runapp"))
  {
    if (!strParameterCaseIntact.IsEmpty())
    {
      CURI appUrl(strParameterCaseIntact);

      if (appUrl.GetHostName() == CAppManager::GetInstance().GetLastLaunchedId())
      {
        CLog::Log(LOGINFO, "CUtil::ExecBuiltIn, runapp asked to run the currently running app.");
      }
      else
      {
        if (g_application.IsPlaying())
        {
          g_application.StopPlaying();
        }

        g_windowManager.CloseDialogs(true);
        while (g_windowManager.GetActiveWindow() >= WINDOW_APPS_START && g_windowManager.GetActiveWindow() < WINDOW_APPS_END)
        {
          g_windowManager.PreviousWindow();
        }

        CAppManager::GetInstance().Launch(strParameterCaseIntact);
      }
    }
    else
    {
      CLog::Log(LOGERROR, "CUtil::ExecBuiltIn, runapp called with no arguments.");
    }
  }  
  else if (execute.Equals("playmedia"))
  {
    if (!params.size())
    {
      CLog::Log(LOGERROR, "XBMC.PlayMedia called with empty parameter");
      return -3;
    }

    CFileItem item(params[0], false);

    // restore to previous window if needed
    if( g_windowManager.GetActiveWindow() == WINDOW_SLIDESHOW ||
        g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO ||
        g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION )
        g_windowManager.PreviousWindow();

    // reset screensaver
    g_application.ResetScreenSaver();
    g_application.WakeUpScreenSaverAndDPMS();

    // set fullscreen or windowed
    if (params.size() >= 2 && params[1] == "1")
      g_stSettings.m_bStartVideoWindowed = true;

#ifndef _BOXEE_
    // ask if we need to check guisettings to resume
    bool askToResume = true;
    if ((params.size() == 2 && params[1].Equals("resume")) || (params.size() == 3 && params[2].Equals("resume")))
    {
      // force the item to resume (if applicable) (see CApplication::PlayMedia)
      item.m_lStartOffset = STARTOFFSET_RESUME;
      askToResume = false;
    }

    if ((params.size() == 2 && params[1].Equals("noresume")) || (params.size() == 3 && params[2].Equals("noresume")))
    {
      // force the item to start at the beginning (m_lStartOffset is initialized to 0)
      askToResume = false;
    }

    if ( askToResume == true )
    {
      if ( CGUIWindowVideoBase::OnResumeShowMenu(item) == false )
        return false;
    }
#endif 
    // play media
    if (!g_application.PlayMedia(item, item.IsAudio() ? PLAYLIST_MUSIC : PLAYLIST_VIDEO))
    {
      CLog::Log(LOGERROR, "XBMC.PlayMedia could not play media: %s", params[0].c_str());
      return false;
    }
  }
  else if (execute.Equals("slideShow") || execute.Equals("recursiveslideShow"))
  {
    if (!params.size())
    {
      CLog::Log(LOGERROR, "XBMC.SlideShow called with empty parameter");
      return -2;
    }
    // leave RecursiveSlideShow command as-is
    unsigned int flags = 0;
    if (execute.Equals("RecursiveSlideShow"))
    {
      flags |= 1;
    }
    else
    {
      if ((params.size() > 1 && params[1] == "recursive") || (params.size() > 2 && params[2] == "recursive"))
        flags |= 1;
      if ((params.size() > 1 && params[1] == "random") || (params.size() > 2 && params[2] == "random"))
        flags |= 2;
      if ((params.size() > 1 && params[1] == "notrandom") || (params.size() > 2 && params[2] == "notrandom"))
        flags |= 4;
    }

    CGUIMessage msg(GUI_MSG_START_SLIDESHOW, 0, 0, flags);
    msg.SetStringParam(params[0]);
    CGUIWindow *pWindow = g_windowManager.GetWindow(WINDOW_SLIDESHOW);
    if (pWindow) pWindow->OnMessage(msg);
  }
  else if (execute.Equals("reloadskin"))
  {
    //  Reload the skin
    g_application.ReloadSkin();
  }
  else if (execute.Equals("refreshrss"))
  {
    g_rssManager.Stop();
    g_settings.LoadRSSFeeds();
    g_rssManager.Start();
  }
  else if (execute.Equals("playercontrol"))
  {
    g_application.ResetScreenSaver();
    g_application.WakeUpScreenSaverAndDPMS();
    if (!params.size())
    {
      CLog::Log(LOGERROR, "XBMC.PlayerControl called with empty parameter");
      return -3;
    }
    if (parameter.Equals("play"))
    { // play/pause
      // either resume playing, or pause
      if (g_application.IsPlaying())
      {
        if (g_application.GetPlaySpeed() != 1)
           g_application.SetPlaySpeed(1);        
        else
           g_application.m_pPlayer->Pause();

        if (!g_application.m_pPlayer->IsPaused())
        {
          CGUIDialogBoxeeVideoCtx* pDlgInfo = (CGUIDialogBoxeeVideoCtx*)g_windowManager.GetWindow(WINDOW_DIALOG_BOXEE_VIDEO_CTX);
          if (pDlgInfo)
            pDlgInfo->Close();

          CGUIDialogBoxeeMusicCtx* pDlgMusicInfo = (CGUIDialogBoxeeMusicCtx*)g_windowManager.GetWindow(WINDOW_DIALOG_BOXEE_MUSIC_CTX);
          if (pDlgMusicInfo)
            pDlgMusicInfo->Close();

          CGUIDialogBoxeePictureCtx* pDlgPicInfo = (CGUIDialogBoxeePictureCtx*)g_windowManager.GetWindow(WINDOW_DIALOG_BOXEE_PICTURE_CTX);
          if (pDlgPicInfo)
            pDlgPicInfo->Close();
        }
      }
    }
    else if (parameter.Equals("stop"))
    {
      g_application.StopPlaying();
    }
    else if (parameter.Equals("rewind") || parameter.Equals("forward"))
    {
      if (g_application.IsPlaying() && !g_application.m_pPlayer->IsPaused())
      {
        int iPlaySpeed = g_application.GetPlaySpeed();
        if (parameter.Equals("rewind") && iPlaySpeed == 1) // Enables Rewinding
          iPlaySpeed *= -2;
        else if (parameter.Equals("rewind") && iPlaySpeed > 1) //goes down a notch if you're FFing
          iPlaySpeed /= 2;
        else if (parameter.Equals("forward") && iPlaySpeed < 1) //goes up a notch if you're RWing
        {
          iPlaySpeed /= 2;
          if (iPlaySpeed == -1) iPlaySpeed = 1;
        }
        else
          iPlaySpeed *= 2;

        if (iPlaySpeed > 32 || iPlaySpeed < -32)
          iPlaySpeed = 1;

        g_application.SetPlaySpeed(iPlaySpeed);
      }
    }
    else if (parameter.Equals("next"))
    {
      CAction action;
      action.id = ACTION_NEXT_ITEM;
      g_application.OnAction(action);
    }
    else if (parameter.Equals("previous"))
    {
      CAction action;
      action.id = ACTION_PREV_ITEM;
      g_application.OnAction(action);
    }
    else if (parameter.Equals("bigskipbackward"))
    {
      if (g_application.IsPlaying())
        g_application.m_pPlayer->Seek(false, true);
    }
    else if (parameter.Equals("bigskipforward"))
    {
      if (g_application.IsPlaying())
        g_application.m_pPlayer->Seek(true, true);
    }
    else if (parameter.Equals("smallskipbackward"))
    {
      if (g_application.IsPlaying())
        g_application.m_pPlayer->Seek(false, false);
    }
    else if (parameter.Equals("smallskipforward"))
    {
      if (g_application.IsPlaying())
        g_application.m_pPlayer->Seek(true, false);
    }
    else if( parameter.Equals("showvideomenu") )
    {
      if( g_application.IsPlaying() && g_application.m_pPlayer )
      {
        CAction action;
        action.amount1 = action.amount2 = action.repeat = 0.0;
        action.buttonCode = 0;
        action.id = ACTION_SHOW_VIDEOMENU;
        g_application.m_pPlayer->OnAction(action);
      }
    }
    else if( parameter.Equals("record") )
    {
      if( g_application.IsPlaying() && g_application.m_pPlayer && g_application.m_pPlayer->CanRecord())
      {
#ifdef HAS_WEB_SERVER
        if (m_pXbmcHttp && g_stSettings.m_HttpApiBroadcastLevel>=1)
          g_application.getApplicationMessenger().HttpApi(g_application.m_pPlayer->IsRecording()?"broadcastlevel; RecordStopping;1":"broadcastlevel; RecordStarting;1");
#endif
        g_application.m_pPlayer->Record(!g_application.m_pPlayer->IsRecording());
      }
    }
#ifndef _BOXEE_
    else if (parameter.Left(9).Equals("partymode"))
    {
      CStdString strXspPath = "";
      //empty param=music, "music"=music, "video"=video, else xsp path
      PartyModeContext context = PARTYMODECONTEXT_MUSIC;
      if (parameter.size() > 9)
      {
        if (parameter.Mid(10).Equals("video)"))
          context = PARTYMODECONTEXT_VIDEO;
        else if (!parameter.Mid(10).Equals("music)"))
        {
          strXspPath = parameter.Mid(10).TrimRight(")");
          context = PARTYMODECONTEXT_UNKNOWN;
        }
      }
      if (g_partyModeManager.IsEnabled())
        g_partyModeManager.Disable();
      else
        g_partyModeManager.Enable(context, strXspPath);
    }
#endif
    else if (parameter.Equals("random")    ||
             parameter.Equals("randomoff") ||
             parameter.Equals("randomon"))
    {
      // get current playlist
      int iPlaylist = g_playlistPlayer.GetCurrentPlaylist();

      // reverse the current setting
      bool shuffled = g_playlistPlayer.IsShuffled(iPlaylist);
      if ((shuffled && parameter.Equals("randomon")) || (!shuffled && parameter.Equals("randomoff")))
        return 0;
      g_playlistPlayer.SetShuffle(iPlaylist, !shuffled);

      // save settings for now playing windows
      switch (iPlaylist)
      {
      case PLAYLIST_MUSIC:
        g_stSettings.m_bMyMusicPlaylistShuffle = g_playlistPlayer.IsShuffled(iPlaylist);
        g_settings.Save();
        break;
      case PLAYLIST_VIDEO:
        g_stSettings.m_bMyVideoPlaylistShuffle = g_playlistPlayer.IsShuffled(iPlaylist);
        g_settings.Save();
      }

      // send message
      CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_RANDOM, 0, 0, iPlaylist, g_playlistPlayer.IsShuffled(iPlaylist));
      g_windowManager.SendThreadMessage(msg);

    }
    else if (parameter.Left(6).Equals("repeat"))
    {
      // get current playlist
      int iPlaylist = g_playlistPlayer.GetCurrentPlaylist();
      PLAYLIST::REPEAT_STATE state = g_playlistPlayer.GetRepeat(iPlaylist);

      if (parameter.Equals("repeatall"))
        state = PLAYLIST::REPEAT_ALL;
      else if (parameter.Equals("repeatone"))
        state = PLAYLIST::REPEAT_ONE;
      else if (parameter.Equals("repeatoff"))
        state = PLAYLIST::REPEAT_NONE;
      else if (state == PLAYLIST::REPEAT_NONE)
        state = PLAYLIST::REPEAT_ALL;
      else if (state == PLAYLIST::REPEAT_ALL)
        state = PLAYLIST::REPEAT_ONE;
      else
        state = PLAYLIST::REPEAT_NONE;

      g_playlistPlayer.SetRepeat(iPlaylist, state);

      // save settings for now playing windows
      switch (iPlaylist)
      {
      case PLAYLIST_MUSIC:
        g_stSettings.m_bMyMusicPlaylistRepeat = (state == PLAYLIST::REPEAT_ALL);
        g_settings.Save();
        break;
      case PLAYLIST_VIDEO:
        g_stSettings.m_bMyVideoPlaylistRepeat = (state == PLAYLIST::REPEAT_ALL);
        g_settings.Save();
      }

      // send messages so now playing window can get updated
      CGUIMessage msg(GUI_MSG_PLAYLISTPLAYER_REPEAT, 0, 0, iPlaylist, (int)state);
      g_windowManager.SendThreadMessage(msg);
    }
  }
  else if (execute.Equals("playwith"))
  {
    g_application.m_eForcedNextPlayer = CPlayerCoreFactory::GetPlayerCore(parameter);
    CAction action;
    action.id = ACTION_PLAYER_PLAY;
    g_application.OnAction(action);
  }
  else if (execute.Equals("mute"))
  {
    g_application.Mute();
  }
  else if (execute.Equals("setvolume"))
  {
    g_application.SetVolume(atoi(parameter.c_str()));
  }
  else if (execute.Equals("volumeup"))
  {
    CAction action;
    action.id = ACTION_VOLUME_UP;
    action.amount1 = 1;
    action.amount2 = false; //false - not originating from cond ACTION_VOLUME_UP_COND
    g_application.OnAction(action);
  }
  else if (execute.Equals("volumedown"))
  {
    CAction action;
    action.id = ACTION_VOLUME_DOWN;
    action.amount1 = 1;
    action.amount2 = false; //false - not originating from cond ACTION_VOLUME_UP_COND
    g_application.OnAction(action);
  }
  else if (execute.Equals("playlist.playoffset"))
  {
    // get current playlist
    int pos = atol(parameter.c_str());
    g_playlistPlayer.PlayNext(pos);
  }
  else if (execute.Equals("playlist.clear"))
  {
    g_playlistPlayer.Clear();
  }
  else if (execute.Equals("ejecttray"))
  {
#ifdef HAS_DVD_DRIVE
    CIoSupport::ToggleTray();
#endif
  }
  else if( execute.Equals("alarmclock") && params.size() > 1 )
  {
    // format is alarmclock(name,command[,seconds,true]);
    float seconds = 0;
    bool silent = false;
    if (params.size() > 2)
      seconds = static_cast<float>(atoi(params[2].c_str())*60);
    else
    { // check if shutdown is specified in particular, and get the time for it
      CStdString strHeading;
      CStdString command;
      vector<CStdString> commandParams;
      CUtil::SplitExecFunction(params[1], command, commandParams);
      if (command.CompareNoCase("shutdown") == 0)
        strHeading = g_localizeStrings.Get(20145);
      else
        strHeading = g_localizeStrings.Get(13209);
      CStdString strTime;
      if( CGUIDialogNumeric::ShowAndGetNumber(strTime, strHeading) )
        seconds = static_cast<float>(atoi(strTime.c_str())*60);
    }
    if (params.size() > 3 && params[3].CompareNoCase("true") == 0)
      silent = true;

    if( g_alarmClock.isRunning() )
      g_alarmClock.stop(params[0]);

    g_alarmClock.start(params[0], seconds, params[1], silent);
  }
  else if (execute.Equals("notification"))
  {
    if (params.size() < 2)
      return -1;
    if (params.size() == 4)
      g_application.m_guiDialogKaiToast.QueueNotification(params[3],params[0],params[1],atoi(params[2].c_str()));
    else if (params.size() == 3)
      g_application.m_guiDialogKaiToast.QueueNotification("",params[0],params[1],atoi(params[2].c_str()));
    else
      g_application.m_guiDialogKaiToast.QueueNotification(params[0],params[1]);
  }
  else if (execute.Equals("cancelalarm"))
  {
    g_alarmClock.stop(parameter);
  }  
  else if (execute.Equals("playdvd"))
  {
#ifdef HAS_DVD_DRIVE    
    CAutorun::PlayDisc();
#endif    
  }
  else if (execute.Equals("skin.togglesetting"))
  {
    int setting = g_settings.TranslateSkinBool(parameter);
    g_settings.SetSkinBool(setting, !g_settings.GetSkinBool(setting));
    g_settings.Save();
  }
  else if (execute.Equals("skin.setbool") && params.size())
  {
    if (params.size() > 1)
    {
      int string = g_settings.TranslateSkinBool(params[0]);
      g_settings.SetSkinBool(string, params[1].CompareNoCase("true") == 0);
      g_settings.Save();
      return 0;
    }
    // default is to set it to true
    int setting = g_settings.TranslateSkinBool(params[0]);
    g_settings.SetSkinBool(setting, true);
    g_settings.Save();
  }
  else if (execute.Equals("skin.reset"))
  {
    g_settings.ResetSkinSetting(parameter);
    g_settings.Save();
  }
  else if (execute.Equals("skin.resetsettings"))
  {
    g_settings.ResetSkinSettings();
    g_settings.Save();
  }
  else if (execute.Equals("skin.theme"))
  {
    // enumerate themes
    vector<CStdString> vecTheme;
    CUtil::GetSkinThemes(vecTheme);

    int iTheme = -1;

    CStdString strTmpTheme;
    // find current theme
    if (!g_guiSettings.GetString("lookandfeel.skintheme").Equals("skindefault"))
      for (unsigned int i=0;i<vecTheme.size();++i)
      {
        strTmpTheme = g_guiSettings.GetString("lookandfeel.skintheme");
        CUtil::RemoveExtension(strTmpTheme);
        if (vecTheme[i].Equals(strTmpTheme))
        {
          iTheme=i;
          break;
        }
      }

    int iParam = atoi(parameter.c_str());
    if (iParam == 0 || iParam == 1)
      iTheme++;
    else if (iParam == -1)
      iTheme--;
    if (iTheme > (int)vecTheme.size()-1)
      iTheme = -1;
    if (iTheme < -1)
      iTheme = vecTheme.size()-1;

    CStdString strSkinTheme;
    if (iTheme==-1)
      g_guiSettings.SetString("lookandfeel.skintheme","skindefault");
    else
    {
      strSkinTheme.Format("%s.xpr",vecTheme[iTheme]);
      g_guiSettings.SetString("lookandfeel.skintheme",strSkinTheme);
    }
    // also set the default color theme
    CStdString colorTheme(strSkinTheme);
    CUtil::ReplaceExtension(colorTheme, ".xml", colorTheme);

    g_guiSettings.SetString("lookandfeel.skincolors", colorTheme);

    g_application.DelayLoadSkin();
  }
  else if (execute.Equals("skin.setstring") || execute.Equals("skin.setimage") || execute.Equals("skin.setfile") ||
           execute.Equals("skin.setpath") || execute.Equals("skin.setnumeric") || execute.Equals("skin.setlargeimage"))
  {
    // break the parameter up if necessary
    int string = 0;
    if (params.size() > 1)
    {
      string = g_settings.TranslateSkinString(params[0]);
      if (execute.Equals("skin.setstring"))
      {
        g_settings.SetSkinString(string, params[1]);
        g_settings.Save();
        return 0;
      }
    }
    else
      string = g_settings.TranslateSkinString(params[0]);
    CStdString value = g_settings.GetSkinString(string);
    VECSOURCES localShares;
    g_mediaManager.GetLocalDrives(localShares);
    if (execute.Equals("skin.setstring"))
    {
      if (CGUIDialogKeyboard::ShowAndGetInput(value, g_localizeStrings.Get(1029), true))
        g_settings.SetSkinString(string, value);
    }
    else if (execute.Equals("skin.setnumeric"))
    {
      if (CGUIDialogNumeric::ShowAndGetNumber(value, g_localizeStrings.Get(611)))
        g_settings.SetSkinString(string, value);
    }
    else if (execute.Equals("skin.setimage"))
    {
      CLog::Log(LOGDEBUG, "Setting image (backg)");
      if (CGUIDialogFileBrowser::ShowAndGetImage(localShares, g_localizeStrings.Get(1030), value))
      {
        CLog::Log(LOGDEBUG, "Setting image, path = %s (backg)", value.c_str());
        g_settings.SetSkinString(string, value);
        g_settings.SetSkinBool(g_settings.TranslateSkinBool("EnableCustomBG"), true);
    }
      else
      {
        //value = g_settings.GetSkinString("CustomBG");
        g_settings.ResetSkinSetting("CustomBG");
        g_settings.ResetSkinSetting("EnableCustomBG");
        g_settings.Save();
      }
    }
    else if (execute.Equals("skin.setlargeimage"))
    {
      VECSOURCES *shares = g_settings.GetSourcesFromType("pictures");
      if (!shares) shares = &localShares;
      if (CGUIDialogFileBrowser::ShowAndGetImage(*shares, g_localizeStrings.Get(1030), value))
        g_settings.SetSkinString(string, value);
    }
    else if (execute.Equals("skin.setfile"))
    {
      CStdString strMask = (params.size() > 1) ? params[1] : "";
    
      if (params.size() > 2)
      {
        value = params[2];
        CUtil::AddSlashAtEnd(value);
        bool bIsSource;
        if (CUtil::GetMatchingSource(value,localShares,bIsSource) < 0) // path is outside shares - add it as a separate one
        {
          CMediaSource share;
          share.strName = g_localizeStrings.Get(13278);
          share.strPath = value;
          localShares.push_back(share);
        }
      }
      if (CGUIDialogFileBrowser::ShowAndGetFile(localShares, strMask, g_localizeStrings.Get(1033), value))
        g_settings.SetSkinString(string, value);
    }
    else // execute.Equals("skin.setpath"))
    {
      if (CGUIDialogFileBrowser::ShowAndGetDirectory(localShares, g_localizeStrings.Get(1031), value))
        g_settings.SetSkinString(string, value);
    }
    g_settings.Save();
  }
  else if (execute.Equals("dialog.close") && params.size())
  {
    bool bForce = false;
    if (params.size() > 1 && params[1].CompareNoCase("true") == 0)
      bForce = true;
    if (params[0].CompareNoCase("all") == 0)
    {
      g_windowManager.CloseDialogs(bForce);
    }
    else
    {
      int id = CButtonTranslator::TranslateWindowString(params[0]);
      CGUIWindow *window = (CGUIWindow *)g_windowManager.GetWindow(id);
      if (window && window->IsDialog())
        ((CGUIDialog *)window)->Close(bForce);
    }
  }
  else if (execute.Equals("system.logoff"))
  {
    if (g_windowManager.GetActiveWindow() == WINDOW_LOGIN_SCREEN || !g_settings.bUseLoginScreen)
      return -1;

    g_settings.m_iLastUsedProfileIndex = g_settings.m_iLastLoadedProfileIndex;
    g_application.StopPlaying();
    CGUIDialogMusicScan *musicScan = (CGUIDialogMusicScan *)g_windowManager.GetWindow(WINDOW_DIALOG_MUSIC_SCAN);
    if (musicScan && musicScan->IsScanning())
      musicScan->StopScanning();

    g_application.getNetwork().NetworkMessage(CNetwork::SERVICES_DOWN,1);
    g_settings.LoadProfile(0); // login screen always runs as default user
    g_passwordManager.m_mapSMBPasswordCache.clear();
    g_passwordManager.bMasterUser = false;
    g_windowManager.ActivateWindow(WINDOW_LOGIN_SCREEN);
    g_application.StartEventServer(); // event server could be needed in some situations
  }
  else if (execute.Equals("pagedown"))
  {
    int id = atoi(parameter.c_str());
    CGUIMessage message(GUI_MSG_PAGE_DOWN, g_windowManager.GetFocusedWindow(), id);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("pageup"))
  {
    int id = atoi(parameter.c_str());
    CGUIMessage message(GUI_MSG_PAGE_UP, g_windowManager.GetFocusedWindow(), id);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("updatelibrary") && params.size())
  {
    if (params[0].Equals("music"))
    {
      CGUIDialogMusicScan *scanner = (CGUIDialogMusicScan *)g_windowManager.GetWindow(WINDOW_DIALOG_MUSIC_SCAN);
      if (scanner)
      {
        if (scanner->IsScanning())
          scanner->StopScanning();
        else
          scanner->StartScanning("");
      }
    }
    if (params[0].Equals("video"))
    {
      CGUIDialogVideoScan *scanner = (CGUIDialogVideoScan *)g_windowManager.GetWindow(WINDOW_DIALOG_VIDEO_SCAN);
      SScraperInfo info;
      //VIDEO::SScanSettings settings;
      if (scanner)
      {
        if (scanner->IsScanning())
          scanner->StopScanning();
#ifndef _BOXEE_
        else
          CGUIWindowVideoBase::OnScan(params.size() > 1 ? params[1] : "",info,settings);
#endif
      }
    }
  }
  else if (execute.Equals("cleanlibrary"))
  {
    CGUIDialogVideoScan *scanner = (CGUIDialogVideoScan *)g_windowManager.GetWindow(WINDOW_DIALOG_VIDEO_SCAN);
    if (scanner)
    {
      if (!scanner->IsScanning())
      {
         CVideoDatabase videodatabase;
         videodatabase.Open();
         videodatabase.CleanDatabase();
         videodatabase.Close();
      }
      else
        CLog::Log(LOGERROR, "XBMC.CleanLibrary is not possible while scanning for media info");
    }
  }
#ifdef HAS_LASTFM
  else if (execute.Equals("lastfm.love"))
  {
    CLastFmManager::GetInstance()->Love(parameter.Equals("false") ? false : true);
  }
  else if (execute.Equals("lastfm.ban"))
  {
    CLastFmManager::GetInstance()->Ban(parameter.Equals("false") ? false : true);
  }
  else if (execute.Equals("lastfm.settings"))
  {
    CLastFmManager::GetInstance()->ShowLastFMSettings();
  }    
#endif
  else if (execute.Equals("control.move") && params.size() > 1)
  {
    CGUIMessage message(GUI_MSG_MOVE_OFFSET, g_windowManager.GetFocusedWindow(), atoi(params[0].c_str()), atoi(params[1].c_str()));
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("container.refresh"))
  { // NOTE: These messages require a media window, thus they're sent to the current activewindow.
    //       This shouldn't stop a dialog intercepting it though.
    CGUIMessage message(GUI_MSG_NOTIFY_ALL, g_windowManager.GetActiveWindow(), 0, GUI_MSG_UPDATE, 1); // 1 to reset the history
    message.SetStringParam(parameter);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("window.refresh"))
  {
    CGUIMessage message(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
    g_windowManager.SendMessage(message, g_windowManager.GetActiveWindow());
  }
  else if (execute.Equals("container.update") && params.size())
  {
    CGUIMessage message(GUI_MSG_NOTIFY_ALL, g_windowManager.GetActiveWindow(), 0, GUI_MSG_UPDATE, 0);
    message.SetStringParam(params[0]);
    if (params.size() > 1 && params[1].CompareNoCase("replace") == 0)
      message.SetParam2(1); // reset the history
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("container.nextviewmode"))
  {
    CGUIMessage message(GUI_MSG_CHANGE_VIEW_MODE, g_windowManager.GetActiveWindow(), 0, 0, 1);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("container.previousviewmode"))
  {
    CGUIMessage message(GUI_MSG_CHANGE_VIEW_MODE, g_windowManager.GetActiveWindow(), 0, 0, -1);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("container.setviewmode"))
  {
    CGUIMessage message(GUI_MSG_CHANGE_VIEW_MODE, g_windowManager.GetActiveWindow(), 0, atoi(parameter.c_str()));
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("container.nextsortmethod"))
  {
    CGUIMessage message(GUI_MSG_CHANGE_SORT_METHOD, g_windowManager.GetActiveWindow(), 0, 0, 1);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("container.previoussortmethod"))
  {
    CGUIMessage message(GUI_MSG_CHANGE_SORT_METHOD, g_windowManager.GetActiveWindow(), 0, 0, -1);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("container.setsortmethod"))
  {
    CGUIMessage message(GUI_MSG_CHANGE_SORT_METHOD, g_windowManager.GetActiveWindow(), 0, atoi(parameter.c_str()));
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("container.focusitem"))
  {
    if (params.size() < 2)
      return -1;
    
    int componentId = atoi(params[0].c_str());
    int item = atoi(params[1].c_str());
    
    CGUIMessage message(GUI_MSG_ITEM_SELECT, g_windowManager.GetActiveWindow(), componentId, item);
    g_windowManager.SendMessage(message);
  }  
  else if (execute.Equals("container.selectitem"))
  {
    if (params.size() < 2)
      return -1;
    
    int componentId = atoi(params[0].c_str());
    int item = atoi(params[1].c_str());
    
    CGUIMessage message(GUI_MSG_MARK_ITEM, g_windowManager.GetActiveWindow(), componentId, item);
    g_windowManager.SendMessage(message);
  }  
  else if (execute.Equals("container.selectfocuseditem"))
  {
    if (params.size() != 1)
      return -1;
    
    int componentId = atoi(params[0].c_str());
    
    CGUIMessage msg1(GUI_MSG_ITEM_SELECTED, g_windowManager.GetActiveWindow(), componentId);
    g_windowManager.SendMessage(msg1);
    int item = msg1.GetParam1();
    
    CGUIMessage message(GUI_MSG_MARK_ITEM, g_windowManager.GetActiveWindow(), componentId, item);
    g_windowManager.SendMessage(message);
  }  
  else if (execute.Equals("container.unselectitem"))
  {
    if (params.size() < 2)
      return -1;
    
    int componentId = atoi(params[0].c_str());
    int item = atoi(params[1].c_str());
    
    CGUIMessage message(GUI_MSG_UNMARK_ITEM, g_windowManager.GetActiveWindow(), componentId, item);
    g_windowManager.SendMessage(message);
  } 
  else if (execute.Equals("container.selectall"))
  {
    if (params.size() != 1)
      return -1;
    
    int componentId = atoi(params[0].c_str());
    
    CGUIMessage message(GUI_MSG_MARK_ALL_ITEMS, g_windowManager.GetActiveWindow(), componentId);
    g_windowManager.SendMessage(message);
  }      
  else if (execute.Equals("container.unselectall"))
  {
    if (params.size() != 1)
      return -1;
    
    int componentId = atoi(params[0].c_str());
    
    CGUIMessage message(GUI_MSG_UNMARK_ALL_ITEMS, g_windowManager.GetActiveWindow(), componentId);
    g_windowManager.SendMessage(message);
  }      
  else if (execute.Equals("container.setpath"))
  {
	  if (params.size() < 2)
		  return -1;
    
    CGUIWindowApp* pWindow = (CGUIWindowApp*) g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
    if (pWindow)
    {
      // Request to load all items
      for (size_t i = 0; i < params.size(); i += 2)
      {
        int componentId = atoi(params[i].c_str());
        CStdString str = params[i+1];
        pWindow->SetContainerPath(componentId, str);
        CLog::Log(LOGDEBUG,"CUtil::ExecBuiltIn - container.setpath(%d,%s)", componentId, str.c_str());
      }
      
      CGUIWindowAppWaitLoading* job = new CGUIWindowAppWaitLoading();
      CUtil::RunInBG(job);
    }
  }
  else if (execute.Equals("savewindowstate"))
  {
    CGUIMessage message(GUI_MSG_SAVE_STATE, g_windowManager.GetActiveWindow(), 0);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("resetwindowstate"))
  {
    CGUIMessage message(GUI_MSG_RESTORE_STATE, g_windowManager.GetActiveWindow(), 0, 1);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("restorewindowstate"))
  {
    CGUIMessage message(GUI_MSG_RESTORE_STATE, g_windowManager.GetActiveWindow(), 0, 0);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("popwindowstate"))
  {
    CGUIMessage message(GUI_MSG_RESET_STATE, g_windowManager.GetActiveWindow(), 0, 0);
    g_windowManager.SendMessage(message);
  }  
  else if (execute.Equals("poptowindowstate"))
  {
    CGUIMessage message(GUI_MSG_RESET_STATE, g_windowManager.GetActiveWindow(), 0, atol(parameter.c_str()));
    g_windowManager.SendMessage(message);
  }  
  else if (execute.Equals("resetdatabase"))
  {
    CLog::Log(LOGDEBUG,"CUtil::ExecBuiltIn, RESET, will remove database and delete thumbnails");
    g_application.DeleteDatabaseAndThumbnails();
  }
  else if (execute.Equals("cleanoldthumbnails"))
  {
    CLog::Log(LOGDEBUG,"CUtil::ExecBuiltIn - [CleanOldThumbnails] -> Going to call g_application.RemoveOldThumbnails with [force=TRUE] (rot)");
    g_application.RemoveOldThumbnails(true,true);
    }
  else if (execute.Equals("container.sortdirection"))
  {
    CGUIMessage message(GUI_MSG_CHANGE_SORT_DIRECTION, g_windowManager.GetActiveWindow(), 0, 0);
    g_windowManager.SendMessage(message);
  }
  else if (execute.Equals("control.message") && params.size() >= 2)
  {
    int controlID = atoi(params[0].c_str());
    int windowID = (params.size() == 3) ? CButtonTranslator::TranslateWindowString(params[2].c_str()) : g_windowManager.GetActiveWindow();
    if (params[1] == "moveup")
      g_windowManager.SendMessage(GUI_MSG_MOVE_OFFSET, windowID, controlID, 1);
    else if (params[1] == "movedown")
      g_windowManager.SendMessage(GUI_MSG_MOVE_OFFSET, windowID, controlID, -1);
    else if (params[1] == "pageup")
      g_windowManager.SendMessage(GUI_MSG_PAGE_UP, windowID, controlID);
    else if (params[1] == "pagedown")
      g_windowManager.SendMessage(GUI_MSG_PAGE_DOWN, windowID, controlID);
    else if (params[1] == "click")
      g_windowManager.SendMessage(GUI_MSG_CLICKED, controlID, windowID);
  }
  else if (execute.Equals("window.setproperty") && params.size() >= 2)
  {
    int windowID = (params.size() == 3) ? CButtonTranslator::TranslateWindowString(params[2].c_str()) : g_windowManager.GetActiveWindow();
    CGUIWindow* pWindow = g_windowManager.GetWindow(windowID);

    CStdString strPropertyName = params[0];
    CStdString strPropertyValue = params[1];

    pWindow->SetProperty(strPropertyName, strPropertyValue);
  }
  else if (execute.Equals("sendclick") && params.size())
  {
    if (params.size() == 2)
    {
      // have a window - convert it
      int windowID = CButtonTranslator::TranslateWindowString(params[0].c_str());
      CGUIMessage message(GUI_MSG_CLICKED, atoi(params[1].c_str()), windowID);
      g_windowManager.SendMessage(message);
    }
    else
    { // single param - assume you meant the active window
      CGUIMessage message(GUI_MSG_CLICKED, atoi(params[0].c_str()), g_windowManager.GetActiveWindow());
      g_windowManager.SendMessage(message);
    }
  }
  else if (execute.Equals("app.setparam"))
  {
    if (params.size() == 2)
    {
      CAppManager::GetInstance().SetLauchedAppParameter(g_windowManager.GetActiveWindow(), params[0], params[1]);
    }
    else if (params.size() == 3 || params.size() == 4)
    {
      bool hidden = false;
      if (params.size() == 4 && params[3] == "true")
      {
        hidden = true;
      }
      
      CStdString value = params[1];
      if (CGUIDialogKeyboard::ShowAndGetInput(value, params[2], true, hidden))
      {
        CAppManager::GetInstance().SetLauchedAppParameter(g_windowManager.GetActiveWindow(), params[0], params[1]);
      }
    }
  }
  else if (execute.Equals("app.setstring"))
  {
    if (params.size() == 2)
    {
      CAppManager::GetInstance().GetRegistry().Set(params[0], params[1]);
    }
    else if (params.size() == 3 || params.size() == 4)
    {
      bool hidden = false;
      if (params.size() == 4 && params[3] == "true")
      {
        hidden = true;
      }
      
      CStdString value = params[1];
      if (CGUIDialogKeyboard::ShowAndGetInput(value, params[2], true, hidden))
      {
        CAppManager::GetInstance().GetRegistry().Set(params[0], value);
      }
    }
  }
  else if (execute.Equals("app.setbool"))
  {
    if (params.size() == 2)
    {
      if (params[1] == "true" || params[1] == "false")
      {
        CAppManager::GetInstance().GetRegistry().Set(params[0], params[1]);
      }
    }
  }
  else if (execute.Equals("app.toggle"))
  {
    if (params.size() == 1)
    {
      CStdString value = CAppManager::GetInstance().GetRegistry().Get(params[0]);
      if (value.CompareNoCase("true")) CAppManager::GetInstance().GetRegistry().Set(params[0], "false");
      else if (value.CompareNoCase("false")) CAppManager::GetInstance().GetRegistry().Set(params[0], "true");
    }
  }
  else if (execute.Equals("app.resetall"))
  {
    CAppManager::GetInstance().GetRegistry().Clear();
  }
  else if (execute.Equals("app.reset"))
  {
    if (params.size() == 1)
    {
      CAppManager::GetInstance().GetRegistry().Unset(params[0]);
    }
  }
  else if (execute.Equals("control.setlabel"))
  {
    if (params.size() == 2)
    {
      CGUIMessage message(GUI_MSG_LABEL_SET, g_windowManager.GetActiveWindow(), atoi(params[0].c_str()));
      message.SetLabel(params[1]);
      g_windowManager.SendMessage(message);
    }
  }
  else if (execute.Equals("app.pushbackstring"))
  {
    if (params.size() == 3)
    {
      CAppManager::GetInstance().GetRegistry().PushBack(params[0], params[1], atoi(params[2].c_str()));
    }
    else if (params.size() == 4)
    {
      CStdString value = params[1];
      if (CGUIDialogKeyboard::ShowAndGetInput(value, params[3], true))
      {
        CAppManager::GetInstance().GetRegistry().PushBack(params[0], value, atoi(params[2].c_str()));
      }
    }
  }
  else if (execute.Equals("app.pushfrontstring"))
  {
    if (params.size() == 3)
    {
      CAppManager::GetInstance().GetRegistry().PushFront(params[0], params[1], atoi(params[2].c_str()));
    }
    else if (params.size() == 4)
    {
      CStdString value = params[1];
      if (CGUIDialogKeyboard::ShowAndGetInput(value, params[3], true))
      {
        CAppManager::GetInstance().GetRegistry().PushFront(params[0], value, atoi(params[2].c_str()));
      }
    }
  }  
  else if (execute.Equals("togglebutton.select"))
  {
    if (params.size() == 1)
    {      
      const CGUIControl* pControl = g_windowManager.GetWindow(g_windowManager.GetActiveWindow())->GetControl(atoi(params[0].c_str()));
      if (pControl && pControl->GetControlType() == CGUIControl::GUICONTROL_TOGGLEBUTTON)
      {
        ((CGUIToggleButtonControl*)pControl)->SetToggleSelect(g_infoManager.TranslateString("true"));
      }
    }
  } 
  else if (execute.Equals("togglebutton.unselect"))
  {
    if (params.size() == 1)
    {      
      const CGUIControl* pControl = g_windowManager.GetWindow(g_windowManager.GetActiveWindow())->GetControl(atoi(params[0].c_str()));
      if (pControl && pControl->GetControlType() == CGUIControl::GUICONTROL_TOGGLEBUTTON)
      {
        ((CGUIToggleButtonControl*)pControl)->SetToggleSelect(g_infoManager.TranslateString("false"));
      }
    }
  }    
  else if (execute.Equals("log.debug"))
  {
    CLog::Log(LOGDEBUG, "%s", strParameterCaseIntact.c_str());
  }
  else if (execute.Equals("log.error"))
  {
    CLog::Log(LOGERROR, "%s", strParameterCaseIntact.c_str());
  }
  else if (execute.Equals("log.warning"))
  {
    CLog::Log(LOGWARNING, "%s", strParameterCaseIntact.c_str());
  }
  else if (execute.Equals("log.info"))
  {
    CLog::Log(LOGINFO, "%s", strParameterCaseIntact.c_str());
  }
  else if (execute.Equals("action") && params.size())
  {
    // try translating the action from our ButtonTranslator
    int actionID;
    if (CButtonTranslator::TranslateActionString(params[0].c_str(), actionID))
    {
      CAction action;
      action.id = actionID;
      action.amount1 = 1.0f;
      if (params.size() == 2)
      { // have a window - convert it and send to it.
        int windowID = CButtonTranslator::TranslateWindowString(params[1].c_str());
        CGUIWindow *window = g_windowManager.GetWindow(windowID);
        if (window)
          window->OnAction(action);
      }
      else // send to our app
        g_application.OnAction(action);
    }
  }
  else if (execute.Equals("setproperty") && params.size() == 2)
  {
    CGUIWindow *window = g_windowManager.GetWindow(g_windowManager.GetActiveWindow());
    if (window)
      window->SetProperty(params[0],params[1]);
  }
#if defined(HAS_LIRC) || defined(HAS_IRSERVERSUITE)
  else if (execute.Equals("lirc.stop"))
  {
    g_RemoteControl.Disconnect();
    //g_RemoteControl.setUsed(false);
  }
  else if (execute.Equals("lirc.start"))
  {
    //g_RemoteControl.setUsed(true);
    g_RemoteControl.Initialize();
  }
#endif
#ifdef HAS_LCD
  else if (execute.Equals("lcd.suspend"))
  {
    g_lcd->Suspend();
  }
  else if (execute.Equals("lcd.resume"))
  {
    g_lcd->Resume();
  }
#endif
  else if (execute.Equals("togglekeyboard"))
  {
    g_application.GetKeyboards().ToggleKeyboards();
  }
  else if (execute.Equals("BrowserFullScreen"))
  {
    bool bFull = (params[0] == "true");
    CAction action;
    action.id = bFull?ACTION_BROWSER_FULL_SCREEN_ON:ACTION_BROWSER_FULL_SCREEN_OFF;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserBack"))
  {
    CAction action;
    action.id = ACTION_BROWSER_BACK;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserForward"))
  {
    CAction action;
    action.id = ACTION_BROWSER_FORWARD;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserPlaybackPlay"))
  {
    if (g_application.m_pPlayer && g_application.m_pPlayer->IsPaused())
      g_application.m_pPlayer->Pause();
  }
  else if (execute.Equals("BrowserPlaybackPause"))
  {
    if (g_application.m_pPlayer && !g_application.m_pPlayer->IsPaused())
      g_application.m_pPlayer->Pause();
  }
  else if (execute.Equals("BrowserPlaybackTogglePause"))
  {
    g_application.m_pPlayer->Pause();
  }
  else if (execute.Equals("BrowserPlaybackSkip"))
  {
    CAction action;
    action.id = ACTION_BROWSER_PLAYBACK_SKIP;
    action.amount1 = 1;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserPlaybackBigSkip"))
  {
    CAction action;
    action.id = ACTION_BROWSER_PLAYBACK_SKIP;
    action.amount1 = 2;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserPlaybackBack"))
  {
    CAction action;
    action.id = ACTION_BROWSER_PLAYBACK_SKIP;
    action.amount1 = -1;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserPlaybackBigBack"))
  {
    CAction action;
    action.id = ACTION_BROWSER_PLAYBACK_SKIP;
    action.amount1 = -2;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserActivateExt"))
  {
    int nExt = atoi(params[0].c_str());
    CAction action;
    action.id = ACTION_BROWSER_EXT;
    action.amount1 = nExt;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserChangeMode"))
  {
    CStdString strMode = params[0];
    strMode.ToLower();
    CAction action;
    action.id = ACTION_BROWSER_SET_MODE;
    action.strAction = strMode;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserNavigate"))
  {
    CAction action;
    action.id = ACTION_BROWSER_NAVIGATE;
    action.strAction = params[0];
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserToggleMode"))
  {
    CAction action;
    action.id = ACTION_BROWSER_TOGGLE_MODE;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->OnAction(action);
  }
  else if (execute.Equals("BrowserToggleQuality"))
  {
    BrowserToggleQualityFunc(); // Deals with the " fatal error C1061: compiler limit : blocks nested too deeply" error in MSVSC.
  }
  else if (execute.Equals("OsdExtClick"))
  {
    int nExt = atoi(params[0].c_str());
    CAction action;
    action.id = ACTION_OSD_EXT_CLICK;
    action.amount1 = nExt;
    g_application.OnAction(action);
  }
  else
    return -1;

  return 0;
}

int CBuiltins::GetRealActiveWindow()
{
  int activeWindow = g_windowManager.GetActiveWindow();
  if (g_windowManager.HasModalDialog())
  {
    // in case of open dialog -> send the action to the dialog
    activeWindow = g_windowManager.GetTopMostModalDialogID();
  }

  return activeWindow;
}
