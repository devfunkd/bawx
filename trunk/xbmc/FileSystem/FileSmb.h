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

// FileSmb.h: interface for the CFileSMB class.

//

//////////////////////////////////////////////////////////////////////



#if !defined(AFX_FILESMB_H__2C4AB5BC_0742_458D_95EA_E9C77BA5663D__INCLUDED_)

#define AFX_FILESMB_H__2C4AB5BC_0742_458D_95EA_E9C77BA5663D__INCLUDED_


#if _MSC_VER > 1000

#pragma once

#endif // _MSC_VER > 1000

#include "IFile.h"
#include "URL.h"
#include "utils/CriticalSection.h"

#define NT_STATUS_CONNECTION_REFUSED long(0xC0000000 | 0x0236)
#define NT_STATUS_INVALID_HANDLE long(0xC0000000 | 0x0008)
#define NT_STATUS_ACCESS_DENIED long(0xC0000000 | 0x0022)
#define NT_STATUS_OBJECT_NAME_NOT_FOUND long(0xC0000000 | 0x0034)
#define NT_STATUS_REQUEST_NOT_ACCEPTED long(0xC0000000 | 0x00d0) // error returned on connection limit
#ifdef _LINUX
#define NT_STATUS_INVALID_COMPUTER_NAME long(0xC0000000 | 0x0122)
#endif

struct _SMBCCTX;
typedef _SMBCCTX SMBCCTX;

class CSMB : public CCriticalSection
{
public:
  CSMB();
  virtual ~CSMB();
  void Init();
  void Deinit(bool idle = false);
  void Purge();
  void PurgeEx(const CURI& url);
#ifdef _LINUX
  void CheckIfIdle();
  void SetActivityTime();
  void AddActiveConnection();
  void AddIdleConnection();
#endif
  static CStdString URLEncode(const CStdString &value);
  static CStdString URLEncode(const CURI &url);

  DWORD ConvertUnixToNT(int error);
private:
  SMBCCTX *m_context;
  CStdString m_strLastHost;
  CStdString m_strLastShare;
#ifdef _LINUX
  int m_OpenConnections;
  int m_LastActive;
  
  CCriticalSection m_connectionCountLock;
#endif  
};

extern CSMB smb;

namespace XFILE
{
class CFileSMB : public IFile
{
public:
  CFileSMB();
  int OpenFile(const CURI &url, CStdString& strAuth);
  virtual ~CFileSMB();
  virtual void Close();
  virtual int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET);
  virtual unsigned int Read(void* lpBuf, int64_t uiBufSize);
  virtual bool Open(const CURI& url);
  virtual bool Exists(const CURI& url);
  virtual int Stat(const CURI& url, struct __stat64* buffer);
  virtual int Stat(struct __stat64* buffer);
  virtual int64_t GetLength();
  virtual int64_t GetPosition();
  virtual int Write(const void* lpBuf, int64_t uiBufSize);
  virtual bool HasFileError(CStdString* err);

  virtual bool OpenForWrite(const CURI& url, bool bOverWrite = false);
  virtual bool Delete(const CURI& url);
  virtual bool Rename(const CURI& url, const CURI& urlnew);
  virtual int GetChunkSize() {return 1;}
  
protected:
  bool FillBuffer();

  CURI m_url;
  bool IsValidFile(const CStdString& strFileName);  
  int64_t m_fileSize;
  __int64 m_filePos;
  int m_fd;
  
  int m_nBufferSize;
  BYTE *m_buffer;
  bool m_has_error;
  CStdString m_error;
  
  CCriticalSection m_connectionLock;
};
}

#endif // !defined(AFX_FILESMB_H__2C4AB5BC_0742_458D_95EA_E9C77BA5663D__INCLUDED_)
