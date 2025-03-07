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

#include "GUIPanelContainer.h"
#include "GUIListItem.h"
#include "utils/GUIInfoManager.h"
#include "utils/log.h"
#include "Application.h"
#include "Key.h"
#include "SingleLock.h"

using namespace std;

CGUIPanelContainer::CGUIPanelContainer(int parentID, int controlId, float posX, float posY, float width, float height, ORIENTATION orientation, int scrollTime, int preloadItems, ListSelectionMode selectionMode)
    : CGUIBaseContainer(parentID, controlId, posX, posY, width, height, orientation, scrollTime, preloadItems, selectionMode)
{
  ControlType = GUICONTAINER_PANEL;
  m_type = VIEW_TYPE_ICON;
  m_itemsPerRow = 1;
  m_memoryTimer.Start();
  m_bShouldFreeMemory = false;
}

CGUIPanelContainer::~CGUIPanelContainer(void)
{
}

void CGUIPanelContainer::Render()
{
  bool bIsScrolling = IsScrolling();

#ifndef OLD_SCROLLING
  if (!bIsScrolling && !m_deferredActions.empty())
  {
    {
      CSingleLock lock(m_actionsLock);
      if (!m_deferredActions.empty())
      {
        CAction action = m_deferredActions.front();
        m_deferredActions.pop();
        g_application.DeferAction(action);
      }
    }
  }
#endif

  ValidateOffset();

  if (m_bInvalidated)
    UpdateLayout();

  if (!m_layout || !m_focusedLayout) return;

  UpdateScrollOffset();

  int offset = (int)(m_scrollOffset / m_layout->Size(m_orientation) );

  int cacheBefore, cacheAfter;
  GetCacheOffsets(cacheBefore, cacheAfter);

  // the following is required to avoid releasing the mem from rows that are rendered partially 
  if (cacheBefore < 2) cacheBefore = 2;
  if (cacheAfter < 2) cacheAfter = 2;

  // Free memory not used on screen at the moment, do this first so there's more memory for the new items.
  // Free only if we're not scrolling up/down, if there was a change in the movement and the time since we last freed the memory was above 2 seconds
  if (m_bShouldFreeMemory && !ScrollingDown() && !ScrollingUp() && m_memoryTimer.GetElapsedSeconds() > 2)
  {
    FreeMemory(CorrectOffset(offset - cacheBefore, 0), CorrectOffset(offset + cacheAfter + m_itemsPerPage + 1, 0));
    m_bShouldFreeMemory = false;
    m_memoryTimer.StartZero();
  }

  bool clip = g_graphicsContext.SetClipRegion(m_posX + m_offsetX, m_posY + m_offsetY, m_width, m_height);
  float pos = (m_orientation == VERTICAL) ? m_posY + m_offsetY : m_posX + m_offsetX;
  float end = (m_orientation == VERTICAL) ? m_posY + m_height : m_posX + m_width;
  pos += (offset - cacheBefore) * m_layout->Size(m_orientation) - m_scrollOffset;
  end += cacheAfter * m_layout->Size(m_orientation);

  float focusedPos = 0;
  unsigned int focusedCol = 0;
  unsigned int focusedRow = 0;
  CGUIListItemPtr focusedItem;
  int current = (offset - cacheBefore) * m_itemsPerRow;
  unsigned int col = 0;
  unsigned int row = 0;
  while (pos < end && m_items.size())
  {
    if (current >= (int)m_items.size())
      break;
    if (current >= 0)
    {
      CGUIListItemPtr item = m_items[current];
      bool focused = (current == m_offset * m_itemsPerRow + m_cursor) && m_bHasFocus;
      // render our item
      if (focused)
      {
        focusedPos = pos;
        focusedCol = col;
        focusedRow = row;
        focusedItem = item;
      }
      else
      {               
        if (m_orientation == VERTICAL)
            RenderItem(m_posX + m_offsetX + col * m_layout->Size(HORIZONTAL), pos, item.get(), row, col, false);
        else
            RenderItem(pos, m_posY + m_offsetY + col * m_layout->Size(VERTICAL), item.get(), row, col, false);
      }
    }
    
    // increment our position
    if (col < (size_t) (m_itemsPerRow - 1))
      col++;
    else
    {
      pos += m_layout->Size(m_orientation);
      col = 0;
      row++;
    }
    current++;
  }
    
  // and render the focused item last (for overlapping purposes)
  if (focusedItem)
  {
    if (m_orientation == VERTICAL)
      RenderItem(m_posX + m_offsetX + focusedCol * m_layout->Size(HORIZONTAL), focusedPos, focusedItem.get(), focusedRow, focusedCol, true);
    else
      RenderItem(focusedPos, m_posY +  m_offsetY + focusedCol * m_layout->Size(VERTICAL), focusedItem.get(), focusedRow, focusedCol, true);
  }

  m_firstRenderAfterLoad = false;
  
  if(clip)
    g_graphicsContext.RestoreClipRegion();

  UpdatePageControl(offset);

  CGUIControl::Render();
}

bool CGUIPanelContainer::OnAction(const CAction &action)
{
  switch (action.id)
  {
  case ACTION_PAGE_UP:
    {
      if (m_offset == 0)
      { // already on the first page, so move to the first item
        SetCursor(0);
      }
      else
      { // scroll up to the previous page
        Scroll( -m_itemsPerPage);
      }
      return true;
    }
    break;
  case ACTION_PAGE_DOWN:
    {
      if ((m_offset + m_itemsPerPage) * m_itemsPerRow >= (int)m_items.size() || (int)m_items.size() < m_itemsPerPage)
      { // already at the last page, so move to the last item.
        SetCursor(m_items.size() - m_offset * m_itemsPerRow - 1);
      }
      else
      { // scroll down to the next page
        Scroll(m_itemsPerPage);
      }
      return true;
    }
    break;
    // smooth scrolling (for analog controls)
  case ACTION_SCROLL_UP:
    {
      m_analogScrollCount += action.amount1 * action.amount1;
      bool handled = false;
      while (m_analogScrollCount > AnalogScrollSpeed())
      {
        handled = true;
        m_analogScrollCount -= AnalogScrollSpeed();
        if (m_offset > 0 )// && m_cursor <= m_itemsPerPage * m_itemsPerRow / 2)
        {
          Scroll(-1);
        }
        else if (m_cursor > 0)
        {
          SetCursor(m_cursor - 1);
        }
      }
      return handled;
    }
    break;
  case ACTION_SCROLL_DOWN:
    {
      m_analogScrollCount += action.amount1 * action.amount1;
      bool handled = false;
      while (m_analogScrollCount > AnalogScrollSpeed())
      {
        handled = true;
        m_analogScrollCount -= AnalogScrollSpeed();
        if ((m_offset + m_itemsPerPage) * m_itemsPerRow < (int)m_items.size())// && m_cursor >= m_itemsPerPage * m_itemsPerRow / 2)
        {
          Scroll(1);
        }
        else if (m_cursor < m_itemsPerPage * m_itemsPerRow - 1 && m_offset * m_itemsPerRow + m_cursor < (int)m_items.size() - 1)
        {
          SetCursor(m_cursor + 1);
        }
      }
      return handled;
    }
    break;
  }
  return CGUIBaseContainer::OnAction(action);
}

bool CGUIPanelContainer::OnMessage(CGUIMessage& message)
{
  if (message.GetControlId() == GetID() )
  {
    if (message.GetMessage() == GUI_MSG_LABEL_RESET)
    {
      SetCursor(0);
      // fall through to base class
    }
    else if (message.GetMessage() == GUI_MSG_ITEM_SELECT)
    {
      if (m_loading)
      {
        m_savedSelected = message.GetParam1();
      }
      else
      {
      SelectItem(message.GetParam1());
      }
      return true;
    }
    else if (message.GetMessage() == GUI_MSG_SETFOCUS)
    {
      if (message.GetParam1() >= 0) // subfocus item is specified, so set the offset appropriately
        SelectItem(message.GetParam1());
    }
    else if (message.GetMessage() == GUI_MSG_PAGE_UP)
    {
      CAction action;
      action.id = ACTION_PAGE_UP;
      return OnAction(action);
    }
    else if (message.GetMessage() == GUI_MSG_PAGE_DOWN)
    {
      CAction action;
      action.id = ACTION_PAGE_DOWN;
      return OnAction(action);
    }
  }
  return CGUIBaseContainer::OnMessage(message);
}

void CGUIPanelContainer::OnLeft()
{
  bool wrapAround = m_controlLeft == GetID() || !(m_controlLeft || m_leftActions.size());
  if (m_orientation == VERTICAL && MoveLeft(wrapAround))
    return;
  if (m_orientation == HORIZONTAL && MoveUp(wrapAround))
    return;
  CGUIControl::OnLeft();
}

void CGUIPanelContainer::OnRight()
{
  bool wrapAround = m_controlRight == GetID() || !(m_controlRight || m_rightActions.size());
  if (m_orientation == VERTICAL && MoveRight(wrapAround))
    return;
  if (m_orientation == HORIZONTAL && MoveDown(wrapAround))
    return;
  return CGUIControl::OnRight();
}

void CGUIPanelContainer::OnUp()
{
  bool wrapAround = m_controlUp == GetID() || !(m_controlUp || m_upActions.size());
  m_bShouldFreeMemory = true;
  if (m_orientation == VERTICAL && MoveUp(wrapAround))
    return;
  if (m_orientation == HORIZONTAL && MoveLeft(wrapAround))
    return;
  CGUIControl::OnUp();
}

void CGUIPanelContainer::OnDown()
{
  bool wrapAround = m_controlDown == GetID() || !(m_controlDown || m_downActions.size());
  m_bShouldFreeMemory = true;
  if (m_orientation == VERTICAL && MoveDown(wrapAround))
    return;
  if (m_orientation == HORIZONTAL && MoveRight(wrapAround))
    return;
  return CGUIControl::OnDown();
}

bool CGUIPanelContainer::MoveDown(bool wrapAround)
{
  if (m_cursor + m_itemsPerRow < m_itemsPerPage * m_itemsPerRow && (m_offset + 1 + m_cursor / m_itemsPerRow) * m_itemsPerRow < (int)m_items.size())
  { // move to last item if necessary
    if ((m_offset + 1)*m_itemsPerRow + m_cursor >= (int)m_items.size())
      SetCursor((int)m_items.size() - 1 - m_offset*m_itemsPerRow);
    else
      SetCursor(m_cursor + m_itemsPerRow);
  }
  else if ((m_offset + 1 + m_cursor / m_itemsPerRow) * m_itemsPerRow < (int)m_items.size())
  { // we scroll to the next row, and move to last item if necessary
    if ((m_offset + 1)*m_itemsPerRow + m_cursor >= (int)m_items.size())
      SetCursor((int)m_items.size() - 1 - (m_offset + 1)*m_itemsPerRow);
    ScrollToOffset(m_offset + 1);
  }
  else if (wrapAround)
  { // move first item in list
    SetCursor(m_cursor % m_itemsPerRow);
    ScrollToOffset(0);
    g_infoManager.SetContainerMoving(GetID(), 1);
  }
  else
    return false;
  return true;
}

bool CGUIPanelContainer::MoveUp(bool wrapAround)
{
  if (m_cursor >= m_itemsPerRow)
    SetCursor(m_cursor - m_itemsPerRow);
  else if (m_offset > 0)
    ScrollToOffset(m_offset - 1);
  else if (wrapAround)
  { // move last item in list in this column
    SetCursor((m_cursor % m_itemsPerRow) + (m_itemsPerPage - 1) * m_itemsPerRow);
    int offset = max((int)GetRows() - m_itemsPerPage, 0);
    // should check here whether cursor is actually allowed here, and reduce accordingly
    if (offset * m_itemsPerRow + m_cursor >= (int)m_items.size())
      SetCursor((int)m_items.size() - offset * m_itemsPerRow - 1);
    ScrollToOffset(offset);
    g_infoManager.SetContainerMoving(GetID(), -1);
  }
  else
    return false;
  return true;
}

bool CGUIPanelContainer::MoveLeft(bool wrapAround)
{
  int col = m_cursor % m_itemsPerRow;
  if (col > 0)
    SetCursor(m_cursor - 1);
  else if (wrapAround)
  { // wrap around
    SetCursor(m_cursor + m_itemsPerRow - 1);
    if (m_offset * m_itemsPerRow + m_cursor >= (int)m_items.size())
      SetCursor((int)m_items.size() - m_offset * m_itemsPerRow);
  }
  else
    return false;
  return true;
}

bool CGUIPanelContainer::MoveRight(bool wrapAround)
{
  int col = m_cursor % m_itemsPerRow;
  if (col + 1 < m_itemsPerRow && m_offset * m_itemsPerRow + m_cursor + 1 < (int)m_items.size())
    SetCursor(m_cursor + 1);
  else if (wrapAround) // move first item in row
    SetCursor(m_cursor - col);
  else
    return false;
  return true;
}

// scrolls the said amount
void CGUIPanelContainer::Scroll(int amount)
{
  // increase or decrease the offset
  int offset = m_offset + amount;
  if (offset > ((int)GetRows() - m_itemsPerPage) * m_itemsPerRow)
  {
    offset = ((int)GetRows() - m_itemsPerPage) * m_itemsPerRow;
  }
  if (offset < 0) offset = 0;
  ScrollToOffset(offset);
}

void CGUIPanelContainer::ValidateOffset()
{ // first thing is we check the range of m_offset
  if (!m_layout) return;
  if (m_offset > (int)GetRows() - m_itemsPerPage)
  {
    m_offset = (int)GetRows() - m_itemsPerPage;
    m_scrollOffset = m_offset * m_layout->Size(m_orientation);
  }
  if (m_offset < 0)
  {
    m_offset = 0;
    m_scrollOffset = 0;
  }
}

void CGUIPanelContainer::SetCursor(int cursor)
{
  // +1 to ensure we're OK if we have a half item
  if (cursor > (m_itemsPerPage + 1)*m_itemsPerRow - 1) cursor = (m_itemsPerPage + 1)*m_itemsPerRow - 1;
  if (cursor < 0) cursor = 0;
  g_infoManager.SetContainerMoving(GetID(), cursor - m_cursor);
  m_cursor = cursor;
}

void CGUIPanelContainer::CalculateLayout()
{
  GetCurrentLayouts();

  if (!m_layout || !m_focusedLayout) return;
  // calculate the number of items to display
  if (m_orientation == HORIZONTAL)
  {
    m_itemsPerRow = (int)(m_height / m_layout->Size(VERTICAL) );
    m_itemsPerPage = (int)(m_width / m_layout->Size(HORIZONTAL) );
  }
  else
  {
    m_itemsPerRow = (int)(m_width / m_layout->Size(HORIZONTAL) );
    m_itemsPerPage = (int)(m_height / m_layout->Size(VERTICAL) );
  }
  if (m_itemsPerRow < 1) m_itemsPerRow = 1;
  if (m_itemsPerPage < 1) m_itemsPerPage = 1;

  // ensure that the scroll offset is a multiple of our size
  m_scrollOffset = m_offset * m_layout->Size(m_orientation);
}

unsigned int CGUIPanelContainer::GetRows() const
{
  assert(m_itemsPerRow > 0);
  return (m_items.size() + m_itemsPerRow - 1) / m_itemsPerRow;
}

float CGUIPanelContainer::AnalogScrollSpeed() const
{
  return 10.0f / m_itemsPerPage;
}

int CGUIPanelContainer::CorrectOffset(int offset, int cursor) const
{
  return offset * m_itemsPerRow + cursor;
}

bool CGUIPanelContainer::SelectItemFromPoint(const CPoint &point)
{
  if (!m_layout)
    return false;

  float sizeX = m_orientation == VERTICAL ? m_layout->Size(HORIZONTAL) : m_layout->Size(VERTICAL);
  float sizeY = m_orientation == VERTICAL ? m_layout->Size(VERTICAL) : m_layout->Size(HORIZONTAL);

  float posY = m_orientation == VERTICAL ? point.y : point.x;
  for (int y = 0; y < m_itemsPerPage + 1; y++) // +1 to ensure if we have a half item we can select it
  {
    float posX = m_orientation == VERTICAL ? point.x : point.y;
    for (int x = 0; x < m_itemsPerRow; x++)
    {
      int item = x + y * m_itemsPerRow;
      if (posX < sizeX && posY < sizeY && item + m_offset < (int)m_items.size())
      { // found
        SetCursor(item);
        return true;
      }
      posX -= sizeX;
    }
    posY -= sizeY;
  }
  return false;
}

bool CGUIPanelContainer::GetCondition(int condition, int data) const
{ // probably only works vertically atm...
  int row = m_cursor / m_itemsPerRow;
  int col = m_cursor % m_itemsPerRow;
  if (m_orientation == HORIZONTAL)
    swap(row, col);
  switch (condition)
  {
  case CONTAINER_ROW:
    return (row == data);
  case CONTAINER_COLUMN:
    return (col == data);
  default:
    return CGUIBaseContainer::GetCondition(condition, data);
  }
}

void CGUIPanelContainer::SelectItem(int item)
{
  // Check that m_offset is valid
  ValidateOffset();
  // only select an item if it's in a valid range
  if (item >= 0 && (item < (int)m_items.size() || item < (int)m_staticItems.size()))
  {
    // Select the item requested
    if (item >= m_offset * m_itemsPerRow && item < (m_offset + m_itemsPerPage) * m_itemsPerRow)
    { // the item is on the current page, so don't change it.
      SetCursor(item - m_offset * m_itemsPerRow);
    }
    else if (item < m_offset * m_itemsPerRow)
    { // item is on a previous page - make it the first item on the page
      SetCursor(item % m_itemsPerRow);
      ScrollToOffset((item - m_cursor) / m_itemsPerRow);
    }
    else // (item >= m_offset+m_itemsPerPage)
    { // item is on a later page - make it the last row on the page
      SetCursor(item % m_itemsPerRow + m_itemsPerRow * (m_itemsPerPage - 1));
      ScrollToOffset((item - m_cursor) / m_itemsPerRow);
    }
  }
}

bool CGUIPanelContainer::HasPreviousPage() const
{
  return (m_offset > 0);
}

bool CGUIPanelContainer::HasNextPage() const
{
  return (m_offset != (int)GetRows() - m_itemsPerPage && (int)GetRows() > m_itemsPerPage);
}

