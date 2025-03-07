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

#include "GUIMediaWindow.h"
#include "FileSystem/VirtualDirectory.h"
#include "FileSystem/DirectoryHistory.h"
#include "GUIWindowSlideShow.h"
#include "PictureThumbLoader.h"
#include "DllImageLib.h"

class CGUIDialogProgress;

class CGUIWindowPictures : public CGUIMediaWindow, public IBackgroundLoaderObserver
{
public:
  CGUIWindowPictures(void);
  virtual ~CGUIWindowPictures(void);
  virtual bool OnMessage(CGUIMessage& message);

  static void Show(const CStdString &strPath="");

protected:
  friend class CGUIWindowBoxeePicturesMain;

  virtual void OnInfo(int item);
  virtual bool OnClick(int iItem);
  virtual void UpdateButtons();
  virtual void OnPrepareFileItems(CFileItemList& items);
  virtual bool Update(const CStdString &strDirectory);
  virtual void GetContextButtons(int itemNumber, CContextButtons &buttons);
  virtual bool OnContextButton(int itemNumber, CONTEXT_BUTTON button);
  virtual void OnFinalizeFileItems(CFileItemList &items);

  void OnRegenerateThumbs();
  virtual bool OnPlayMedia(int iItem);
  bool ShowPicture(int iItem, bool startSlideShow);
  void OnShowPictureRecursive(const CStdString& strPath);
  void OnSlideShow(const CStdString& strPicture);
  void OnSlideShow();
  void OnSlideShowRecursive(const CStdString& strPicture);
  void OnSlideShowRecursive();
  virtual void OnItemLoaded(CFileItem* pItem);
  virtual void LoadPlayList(const CStdString& strPlayList);

  CGUIDialogProgress* m_dlgProgress;
  DllImageLib m_ImageLib;

  CPictureThumbLoader *m_thumbLoader;
};
