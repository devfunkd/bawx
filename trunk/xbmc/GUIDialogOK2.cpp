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


#include "GUIDialogOK2.h"
#include "GUIWindowManager.h"

#define ID_BUTTON_OK   10

CGUIDialogOK2::CGUIDialogOK2(void)
    : CGUIDialogBoxBase(WINDOW_DIALOG_OK_2, "DialogOK2.xml")
{
}

CGUIDialogOK2::~CGUIDialogOK2(void)
{}

bool CGUIDialogOK2::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      if (iControl == ID_BUTTON_OK)
      {
        m_bConfirmed = true;
        Close();
        return true;
      }
    }
    break;
  }
  return CGUIDialogBoxBase::OnMessage(message);
}

// \brief Show CGUIDialogOK dialog, then wait for user to dismiss it.
void CGUIDialogOK2::ShowAndGetInput(int heading, int line)
{
  CGUIDialogOK2 *dialog = (CGUIDialogOK2 *)g_windowManager.GetWindow(WINDOW_DIALOG_OK_2);
  if (!dialog) return;
  dialog->SetHeading( heading );
  dialog->SetLine( 0, line );
  dialog->DoModal();
}

void CGUIDialogOK2::ShowAndGetInput(const CStdString&  heading, const CStdString& line)
{
  CGUIDialogOK2 *dialog = (CGUIDialogOK2 *)g_windowManager.GetWindow(WINDOW_DIALOG_OK_2);
  if (!dialog) return;
  dialog->SetHeading( heading );
  dialog->SetLine( 0, line );
  dialog->DoModal();
}
