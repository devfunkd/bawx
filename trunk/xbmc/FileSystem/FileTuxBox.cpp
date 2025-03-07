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

#include "FileTuxBox.h"

#ifdef HAS_FILESYSTEM_TUXBOX

#include <errno.h>

//Reserved for TuxBox Recording!

using namespace XFILE;

CFileTuxBox::CFileTuxBox()
{}

CFileTuxBox::~CFileTuxBox()
{
}

int64_t CFileTuxBox::GetPosition()
{
  return 0;
}

int64_t CFileTuxBox::GetLength()
{
  return 0;
}

bool CFileTuxBox::Open(const CURI& url)
{
  return true;
}

unsigned int CFileTuxBox::Read(void* lpBuf, int64_t uiBufSize)
{
  return 0;
}

int64_t CFileTuxBox::Seek(int64_t iFilePosition, int iWhence)
{
  return 0;
}

void CFileTuxBox::Close()
{
}

bool CFileTuxBox::Exists(const CURI& url)
{
  return true;
}

int CFileTuxBox::Stat(const CURI& url, struct __stat64* buffer)
{
  errno = ENOENT;
  return -1;
}

#endif
