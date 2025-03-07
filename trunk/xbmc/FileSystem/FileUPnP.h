/*
* UPnP Support for XBMC
* Based on CFileDAAP
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
//
//////////////////////////////////////////////////////////////////////

#ifndef __FILEUPNP_H__
#define __FILEUPNP_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FileCurl.h"
#include "URL.h"
#include "IFile.h"

namespace XFILE
{
class CFileUPnP : public IFile
{
public:
  CFileUPnP();
  virtual ~CFileUPnP();
  virtual int64_t GetPosition();
  virtual int64_t GetLength();
  virtual bool Open(const CURI& url);
  virtual bool Exists(const CURI& url);
  virtual int Stat(const CURI& url, struct __stat64* buffer);
  virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
  virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
  virtual void Close();

protected:

  bool StartStreaming();
  bool StopStreaming();

  bool m_bOpened;
  
  IFile* m_file; // the underlying resource file
};
}

#endif // __FILEUPNP_H__


