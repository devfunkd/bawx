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

#include "GUIFadeLabelControl.h"
#include "utils/CharsetConverter.h"

using namespace std;

CGUIFadeLabelControl::CGUIFadeLabelControl(int parentID, int controlID, float posX, float posY, float width, float height, const CLabelInfo& labelInfo, bool scrollOut, int scrollSpeed, unsigned int timeToDelayAtEnd, bool resetOnLabelChange)
    : CGUIControl(parentID, controlID, posX, posY, width, height), m_scrollInfo(50, labelInfo.offsetX, scrollSpeed)
    , m_textLayout(labelInfo.font, false)
{
  m_label = labelInfo;
  m_currentLabel = 0;
  ControlType = GUICONTROL_FADELABEL;
  m_scrollOut = scrollOut;
  m_fadeAnim = CAnimation::CreateFader(100, 0, timeToDelayAtEnd, 200);
  if (m_fadeAnim)
    m_fadeAnim->ApplyAnimation();
  m_renderTime = 0;
  m_lastLabel = -1;
  m_scrollSpeed = scrollSpeed;  // save it for later
  m_resetOnLabelChange = resetOnLabelChange;
  m_shortText = false;
}

CGUIFadeLabelControl::CGUIFadeLabelControl(const CGUIFadeLabelControl &from)
: CGUIControl(from), m_infoLabels(from.m_infoLabels), m_scrollInfo(from.m_scrollInfo), m_textLayout(from.m_textLayout)
{
  m_label = from.m_label;
  m_scrollOut = from.m_scrollOut;
  m_scrollSpeed = from.m_scrollSpeed;
  m_resetOnLabelChange = from.m_resetOnLabelChange;

  if (from.m_fadeAnim)
    m_fadeAnim = new CAnimation(*from.m_fadeAnim);
  if (m_fadeAnim)
    m_fadeAnim->ApplyAnimation();
  m_currentLabel = 0;
  m_renderTime = 0;
  m_lastLabel = -1;
  ControlType = GUICONTROL_FADELABEL;
}

CGUIFadeLabelControl::~CGUIFadeLabelControl(void)
{
    delete m_fadeAnim;
}

void CGUIFadeLabelControl::SetInfo(const vector<CGUIInfoLabel> &infoLabels)
{
  m_lastLabel = -1;
  m_infoLabels = infoLabels;
}

void CGUIFadeLabelControl::AddLabel(const string &label)
{
  m_infoLabels.push_back(CGUIInfoLabel(label));
}

void CGUIFadeLabelControl::DoRender(unsigned int currentTime)
{
  m_renderTime = currentTime;
  CGUIControl::DoRender(currentTime);
}

void CGUIFadeLabelControl::UpdateColors()
{
  m_label.UpdateColors();
  CGUIControl::UpdateColors();
}

void CGUIFadeLabelControl::Render()
{
  if (m_infoLabels.size() == 0)
  { // nothing to render
    CGUIControl::Render();
    return ;
  }

  if (m_currentLabel >= m_infoLabels.size() )
    m_currentLabel = 0;

  if (m_textLayout.Update(m_infoLabels[m_currentLabel].GetLabel(m_parentID)))
  { // changed label - update our suffix based on length of available text
    float width, height;
    m_textLayout.GetTextExtent(width, height);
    float spaceWidth = m_label.font->GetCharWidth(L' ');
    unsigned int numSpaces = (unsigned int)(m_width / spaceWidth) + 1;
    if (width < m_width) // append spaces for scrolling
      numSpaces += (unsigned int)((m_width - width) / spaceWidth) + 1;
    m_shortText = (width + m_label.offsetX) < m_width;
    m_scrollInfo.suffix.assign(numSpaces, L' ');
    if (m_resetOnLabelChange)
    {
      m_scrollInfo.Reset();
      m_fadeAnim->ResetAnimation();
  }
  }
  if (m_currentLabel != m_lastLabel)
  { // new label - reset scrolling
    m_scrollInfo.Reset();
    m_fadeAnim->QueueAnimation(ANIM_PROCESS_REVERSE);
    m_lastLabel = m_currentLabel;
  }

  float posY = m_posY;
  if (m_label.align & XBFONT_CENTER_Y)
    posY += m_height * 0.5f;
  if (m_infoLabels.size() == 1 && m_shortText) 
  { // single label set and no scrolling required - just display
    m_textLayout.Render(m_posX + m_label.offsetX, posY, 0, m_label.textColor, m_label.shadowColor, (m_label.align & ~3), m_width - m_label.offsetX);
    CGUIControl::Render();
    return;
  }

  bool moveToNextLabel = false;
  if (!m_scrollOut)
  {
    vecText text;
    m_textLayout.GetFirstText(text);
    if (m_scrollInfo.characterPos && m_scrollInfo.characterPos < text.size())
      text.erase(text.begin(), text.begin() + min((int)m_scrollInfo.characterPos - 1, (int)text.size()));
    if (m_label.font->GetTextWidth(text) < m_width)
    {
      if (m_fadeAnim->GetProcess() != ANIM_PROCESS_NORMAL)
        m_fadeAnim->QueueAnimation(ANIM_PROCESS_NORMAL);
      moveToNextLabel = true;
    }
  }
  else if (m_scrollInfo.characterPos > m_textLayout.GetTextLength())
    moveToNextLabel = true;

  // apply the fading animation
  TransformMatrix matrix;
  m_fadeAnim->Animate(m_renderTime, true);
  m_fadeAnim->RenderAnimation(matrix);
  g_graphicsContext.PushTransform(matrix);

  if (m_fadeAnim->GetState() == ANIM_STATE_APPLIED)
    m_fadeAnim->ResetAnimation();

  m_scrollInfo.SetSpeed((m_fadeAnim->GetProcess() == ANIM_PROCESS_NONE) ? m_scrollSpeed : 0);

  if (!m_scrollOut && m_shortText)
  {
    float posX = m_posX + m_label.offsetX;
    if (m_label.align & XBFONT_CENTER_X)
      posX = m_posX + m_width * 0.5f;
    else if (m_label.align & XBFONT_RIGHT)
      posX = m_posX + m_width;
    m_textLayout.Render(posX, posY, 0, m_label.textColor, m_label.shadowColor, m_label.align, m_width);
  }
  else
    m_textLayout.RenderScrolling(m_posX, posY, 0, m_label.textColor, m_label.shadowColor, (m_label.align & ~3), m_width, m_scrollInfo);

  if (moveToNextLabel)
  { // increment the label and reset scrolling
    if (m_fadeAnim->GetProcess() != ANIM_PROCESS_NORMAL)
    {
      unsigned int label = m_currentLabel;
      do
      { // cycle until we get a non-empty label, or stick with the one we have
        label++;
        if (label >= m_infoLabels.size())
          label = 0;
      }
      while (label != m_currentLabel && m_infoLabels[label].GetLabel(m_parentID).IsEmpty());
      m_currentLabel = label;
      m_scrollInfo.Reset();
      m_fadeAnim->QueueAnimation(ANIM_PROCESS_REVERSE);
    }
  }

  g_graphicsContext.PopTransform();

  CGUIControl::Render();
}


bool CGUIFadeLabelControl::CanFocus() const
{
  return false;
}


bool CGUIFadeLabelControl::OnMessage(CGUIMessage& message)
{
  if ( message.GetControlId() == GetID() )
  {
    if (message.GetMessage() == GUI_MSG_LABEL_ADD)
    {
      AddLabel(message.GetLabel());
    }
    if (message.GetMessage() == GUI_MSG_LABEL_RESET)
    {
      m_lastLabel = -1;
      m_infoLabels.clear();
      m_scrollInfo.Reset();
    }
    if (message.GetMessage() == GUI_MSG_LABEL_SET)
    {
      m_lastLabel = -1;
      m_infoLabels.clear();
      m_scrollInfo.Reset();
      AddLabel(message.GetLabel());
    }
  }
  return CGUIControl::OnMessage(message);
}

