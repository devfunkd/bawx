#ifndef _ENCODER_H
#define _ENCODER_H

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

#include "StdString.h"

#define ENC_ARTIST  11
#define ENC_TITLE   12
#define ENC_ALBUM   13
#define ENC_YEAR    14
#define ENC_COMMENT 15
#define ENC_TRACK   16
#define ENC_GENRE   17

#define WRITEBUFFER_SIZE 131072 // 128k buffer

namespace XFILE { class CFile; }

class CEncoder
{
public:
  CEncoder();
  virtual ~CEncoder();
  virtual bool Init(const char* strFile, int iInChannels, int iInRate, int iInBits) = 0;
  virtual int Encode(int nNumBytesRead, uint8_t* pbtStream) = 0;
  virtual bool Close() = 0;

  void SetComment(const char* str) { m_strComment = str; }
  void SetArtist(const char* str) { m_strArtist = str; }
  void SetTitle(const char* str) { m_strTitle = str; }
  void SetAlbum(const char* str) { m_strAlbum = str; }
  void SetGenre(const char* str) { m_strGenre = str; }
  void SetTrack(const char* str) { m_strTrack = str; }
  void SetYear(const char* str) { m_strYear = str; }

protected:
  bool FileCreate(const char* filename);
  bool FileClose();
  int FileWrite(const void *pBuffer, uint32_t iBytes);

  int WriteStream(const void *pBuffer, uint32_t iBytes);
  int FlushStream();

  // tag info
  CStdString m_strComment;
  CStdString m_strArtist;
  CStdString m_strTitle;
  CStdString m_strAlbum;
  CStdString m_strGenre;
  CStdString m_strTrack;
  CStdString m_strYear;

  CStdString m_strFile;

  XFILE::CFile *m_file;
  int m_iInChannels;
  int m_iInSampleRate;
  int m_iInBitsPerSample;

  uint8_t m_btWriteBuffer[WRITEBUFFER_SIZE]; // 128k buffer for writing to disc
  uint32_t m_dwWriteBufferPointer;
};

#endif // _ENCODER_H

