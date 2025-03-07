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

#include "MouseStat.h"
#include "Key.h"
#include "GraphicContext.h"
#include "WindowingFactory.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "utils/AnnouncementManager.h"
#include "GUISettings.h"

CMouseStat g_Mouse;

CMouseStat::CMouseStat()
{
  m_exclusiveWindowID = WINDOW_INVALID;
  m_exclusiveControlID = WINDOW_INVALID;
  m_pointerState = MOUSE_STATE_NORMAL;
  SetEnabled();
  m_speedX = m_speedY = 0;
  m_maxX = m_maxY = 0;
  m_lastEventTime = 0;
  m_lastActiveTime = 0;
  memset(&m_mouseState, 0, sizeof(m_mouseState));
}

CMouseStat::~CMouseStat()
{
}

void CMouseStat::Initialize(void *appData)
{
  // Set the default resolution (PAL)
  SetResolution(720, 576, 1, 1);
  
}

void CMouseStat::Cleanup()
{
}

void CMouseStat::ResetClicks()
{
  for (int i=0; i< (int)MOUSE_BUTTON_LAST; i++)
  {
    bClick[i]=false;
    bDoubleClick[i]=false;
  }
}

void CMouseStat::HandleEvent(XBMC_Event& newEvent)
{
  int dx = 0;
  int dy = 0;
  m_lastEventTime = CTimeUtils::GetFrameTime();
  
  if (m_mouseState.x == -1 || m_mouseState.y == -1)
  {
    m_mouseState.x = newEvent.motion.x;
    m_mouseState.y = newEvent.motion.y;
  }
  else
  {
    dx = newEvent.motion.x - m_mouseState.x;
    dy = newEvent.motion.y - m_mouseState.y;
    m_mouseState.dx = dx;
    m_mouseState.dy = dy;
  }
  
  m_mouseState.x  = std::max(0, std::min(m_maxX, m_mouseState.x + dx));
  m_mouseState.y  = std::max(0, std::min(m_maxY, m_mouseState.y + dy));
  
  // Fill in the public members
  if (newEvent.button.type == XBMC_MOUSEBUTTONDOWN)
  {
    if (newEvent.button.button == XBMC_BUTTON_LEFT) m_mouseState.button[MOUSE_LEFT_BUTTON] = true;
    if (newEvent.button.button == XBMC_BUTTON_RIGHT) m_mouseState.button[MOUSE_RIGHT_BUTTON] = true;
    if (newEvent.button.button == XBMC_BUTTON_MIDDLE) m_mouseState.button[MOUSE_MIDDLE_BUTTON] = true;
    if (newEvent.button.button == XBMC_BUTTON_X1) m_mouseState.button[MOUSE_EXTRA_BUTTON1] = true;
    if (newEvent.button.button == XBMC_BUTTON_X2) m_mouseState.button[MOUSE_EXTRA_BUTTON2] = true;
    if (newEvent.button.button == XBMC_BUTTON_WHEELUP) m_mouseState.dz = 1;
    if (newEvent.button.button == XBMC_BUTTON_WHEELDOWN) m_mouseState.dz = -1;
  }
  else if (newEvent.button.type == XBMC_MOUSEBUTTONUP)
  {
    if (newEvent.button.button == XBMC_BUTTON_LEFT) m_mouseState.button[MOUSE_LEFT_BUTTON] = false;
    if (newEvent.button.button == XBMC_BUTTON_RIGHT) m_mouseState.button[MOUSE_RIGHT_BUTTON] = false;
    if (newEvent.button.button == XBMC_BUTTON_MIDDLE) m_mouseState.button[MOUSE_MIDDLE_BUTTON] = false;
    if (newEvent.button.button == XBMC_BUTTON_X1) m_mouseState.button[MOUSE_EXTRA_BUTTON1] = false;
    if (newEvent.button.button == XBMC_BUTTON_X2) m_mouseState.button[MOUSE_EXTRA_BUTTON2] = false;
    if (newEvent.button.button == XBMC_BUTTON_WHEELUP) m_mouseState.dz = 0;
    if (newEvent.button.button == XBMC_BUTTON_WHEELDOWN) m_mouseState.dz = 0;
  }
  UpdateInternal();
}


void CMouseStat::UpdateInternal()
{
  uint32_t now = CTimeUtils::GetFrameTime();
  // update our state from the mouse device
  if (HasMoved() || m_mouseState.dz)
    SetActive();

  // Perform the click mapping (for single + double click detection)
  bool bNothingDown = true;
  
  for (int i = 0; i < 5; i++)
  {
    bClick[i] = false;
    bDoubleClick[i] = false;
    bHold[i] = false;
    if (m_mouseState.button[i])
    {
      bNothingDown = false;
      SetActive();
      if (m_lastDown[i])
      { // start of hold
        bHold[i] = true;
      }
      else
      {
        if (now - m_lastClickTime[i] < MOUSE_DOUBLE_CLICK_LENGTH)
        { // Double click
          bDoubleClick[i] = true;
        }
      }
    }
    else
    {
      if (m_lastDown[i])
      { // Mouse up
        bNothingDown = false;
        bClick[i] = true;
        m_lastClickTime[i] = now;
      }
    }
    m_lastDown[i] = m_mouseState.button[i];
  }

  if (bNothingDown)
    SetState(MOUSE_STATE_NORMAL);
}


void CMouseStat::SetResolution(int maxX, int maxY, float speedX, float speedY)
{
  m_maxX = maxX;
  m_maxY = maxY;

  // speed is currently unused
  m_speedX = speedX;
  m_speedY = speedY;

  // reset the coordinates
  m_mouseState.x = -1;
  m_mouseState.y = -1;
}

void CMouseStat::SetActive(bool active /*=true*/)
{
  m_lastActiveTime = CTimeUtils::GetFrameTime();
  m_mouseState.active = active;
   // we show the OS mouse if:
  // 1. The mouse is active (it has been moved) AND
  // 2. The XBMC mouse is disabled in settings AND
  // 3. XBMC is not in fullscreen.
#ifdef _WIN32
  g_Windowing.ShowOSMouse(m_mouseState.active && !IsEnabled() && !g_Windowing.IsFullScreen());
#else
  SDL_ShowCursor(m_mouseState.active && !(IsEnabled() || g_Windowing.IsFullScreen()));
#endif
}

// IsActive - returns true if we have been active in the last MOUSE_ACTIVE_LENGTH period
bool CMouseStat::IsActive()
{
  if (m_mouseState.active && (CTimeUtils::GetFrameTime() - m_lastActiveTime > MOUSE_ACTIVE_LENGTH))
    SetActive(false);

  return (m_mouseState.active && IsEnabled());
}

void CMouseStat::SetEnabled(bool enabled)
{
  if (m_mouseEnabled == enabled)
    return;

  m_mouseEnabled = enabled;

  if (enabled)
    ANNOUNCEMENT::CAnnouncementManager::Announce(ANNOUNCEMENT::AF_GUI, "xbmc", "MouseEnabled");
  else
    ANNOUNCEMENT::CAnnouncementManager::Announce(ANNOUNCEMENT::AF_GUI, "xbmc", "MouseDisabled");
}

// IsEnabled - returns true if mouse is enabled
bool CMouseStat::IsEnabled() const
{
  return m_mouseEnabled;
}

bool CMouseStat::HasMoved(bool detectAllMoves /* = false */) const
{
  if (CTimeUtils::GetFrameTime() - m_lastEventTime > MOUSE_MOVE_VALIDITY)
    return false;
  
  if (detectAllMoves)
    return m_mouseState.dx || m_mouseState.dy;
  return (m_mouseState.dx * m_mouseState.dx + m_mouseState.dy * m_mouseState.dy >= MOUSE_MINIMUM_MOVEMENT * MOUSE_MINIMUM_MOVEMENT);
}

CPoint CMouseStat::GetLocation() const
{
  return CPoint((float)m_mouseState.x, (float)m_mouseState.y);
}

void CMouseStat::SetLocation(const CPoint &point, bool activate)
{
  m_mouseState.x = (int)point.x;
  m_mouseState.y = (int)point.y;
  SetActive();
}

CPoint CMouseStat::GetLastMove() const
{
  return CPoint(m_mouseState.dx, m_mouseState.dy);
}

char CMouseStat::GetWheel() const
{
  return m_mouseState.dz;
}

void CMouseStat::UpdateMouseWheel(char dir)
{
  m_mouseState.dz = dir;
  SetActive();
}

void CMouseStat::SetExclusiveAccess(int controlID, int windowID, const CPoint &point)
{
  m_exclusiveControlID = controlID;
  m_exclusiveWindowID = windowID;
  // convert posX, posY to screen coords...
  // NOTE: This relies on the window resolution having been set correctly beforehand in CGUIWindow::OnMouseAction()
  CPoint mouseCoords(GetLocation());
  g_graphicsContext.MapScreenToWorld(mouseCoords.x, mouseCoords.y);
  m_exclusiveOffset = point - mouseCoords;
}

void CMouseStat::EndExclusiveAccess(int controlID, int windowID)
{
  if (m_exclusiveControlID == controlID && m_exclusiveWindowID == windowID)
    SetExclusiveAccess(WINDOW_INVALID, WINDOW_INVALID, CPoint(0, 0));
}

void CMouseStat::Acquire()
{
}

bool CMouseStat::IsEnabledInSettings()
{
  return g_guiSettings.GetBool("lookandfeel.enablemouse");
}
