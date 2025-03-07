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

#include "GUIDialogPictureInfo.h"
#include "utils/GUIInfoManager.h"
#include "Application.h"
#include "GUIWindowManager.h"
#include "FileItem.h"
#include "GUIDialogBoxeeShare.h"
#include "GUIDialogBoxeeRate.h"
#include "BoxeeUtils.h"
#include "LocalizeStrings.h"

#define CONTROL_PICTURE_INFO 5

#define SLIDE_STRING_BASE 21800 - SLIDE_INFO_START

#define BTN_RATE      9005
#define BTN_RECOMMEND 9006

CGUIDialogPictureInfo::CGUIDialogPictureInfo(void)
    : CGUIDialog(WINDOW_DIALOG_PICTURE_INFO, "DialogPictureInfo.xml")
{
  m_pictureInfo = new CFileItemList;
}

CGUIDialogPictureInfo::~CGUIDialogPictureInfo(void)
{
  delete m_pictureInfo;
}

void CGUIDialogPictureInfo::SetPicture(CFileItem *item)
{
  g_infoManager.SetCurrentSlide(*item);
}

void CGUIDialogPictureInfo::OnInitWindow()
{
  CGUIDialog::OnInitWindow();
  UpdatePictureInfo();
}

bool CGUIDialogPictureInfo::OnAction(const CAction& action)
{
  switch (action.id)
  {
    // if we're running from slideshow mode, drop the "next picture" and "previous picture" actions through.
    case ACTION_NEXT_PICTURE:
    case ACTION_PREV_PICTURE:
    case ACTION_PLAYER_PLAY:
    case ACTION_PAUSE:
      if (g_windowManager.GetActiveWindow() == WINDOW_SLIDESHOW)
      {
        CGUIWindow* pWindow = g_windowManager.GetWindow(WINDOW_SLIDESHOW);
        return pWindow->OnAction(action);
      }
      break;
  };
  return CGUIDialog::OnAction(action);
}

void CGUIDialogPictureInfo::Render()
{
  if (g_infoManager.GetCurrentSlide().m_strPath != m_currentPicture)
  {
    UpdatePictureInfo();
    m_currentPicture = g_infoManager.GetCurrentSlide().m_strPath;
  }
  CGUIDialog::Render();
}

bool CGUIDialogPictureInfo::OnMessage(CGUIMessage &message)
{
  switch (message.GetMessage())
  {
	case GUI_MSG_CLICKED:
	{
		unsigned int iControl = message.GetSenderId();
		if (iControl == BTN_RATE)
		{
			bool bLike;
			if (CGUIDialogBoxeeRate::ShowAndGetInput(bLike))
			{
				BoxeeUtils::Rate(&g_infoManager.GetCurrentSlide(), bLike);
				g_application.m_guiDialogKaiToast.QueueNotification(CGUIDialogKaiToast::ICON_STAR, "", g_localizeStrings.Get(51034), 5000 , KAI_YELLOW_COLOR, KAI_GREY_COLOR);
			}
		}
		else if (iControl == BTN_RECOMMEND)
		{
			CGUIDialogBoxeeShare *pFriends = (CGUIDialogBoxeeShare *)g_windowManager.GetWindow(WINDOW_DIALOG_BOXEE_SHARE);
			pFriends->DoModal();
		}
	}
  }
	return CGUIDialog::OnMessage(message);
}

void CGUIDialogPictureInfo::UpdatePictureInfo()
{
  // add stuff from the current slide to the list
  CGUIMessage msgReset(GUI_MSG_LABEL_RESET, GetID(), CONTROL_PICTURE_INFO);
  OnMessage(msgReset);
  m_pictureInfo->Clear();
  for (int info = SLIDE_INFO_START; info <= SLIDE_INFO_END; ++info)
  {
    CStdString picInfo = g_infoManager.GetLabel(info);
    if (!picInfo.IsEmpty())
    {
      CFileItemPtr item(new CFileItem(g_localizeStrings.Get(SLIDE_STRING_BASE + info)));
      item->SetLabel2(picInfo);
      m_pictureInfo->Add(item);
      CGUIMessage msg(GUI_MSG_LABEL_ADD, GetID(), CONTROL_PICTURE_INFO, 0, 0, item);
      OnMessage(msg);
    }
  }
}

void CGUIDialogPictureInfo::OnDeinitWindow(int nextWindowID)
{
  CGUIDialog::OnDeinitWindow(nextWindowID);
  CGUIMessage msgReset(GUI_MSG_LABEL_RESET, GetID(), CONTROL_PICTURE_INFO);
  OnMessage(msgReset);
  m_pictureInfo->Clear();
  m_currentPicture.Empty();
}
