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

#if (defined HAVE_CONFIG_H) && (!defined WIN32)
  #include "config.h"
#endif
#if (defined USE_EXTERNAL_LIBVORBIS)
  #include <vorbis/vorbisenc.h>
  #include "utils/log.h"
#else
  #include "cdrip/oggvorbis/vorbisenc.h"
#endif
#include "DynamicDll.h"

class DllVorbisEncInterface
{
public:
  virtual int vorbis_encode_init(vorbis_info *vi, long channels, long rate, long max_bitrate, long nominal_bitrate, long min_bitrate)=0;
  virtual int vorbis_encode_init_vbr(vorbis_info *vi, long channels, long rate, float base_quality)=0;
  virtual ~DllVorbisEncInterface() {}
};

#if (defined USE_EXTERNAL_LIBVORBIS)

class DllVorbisEnc : public DllDynamic, DllVorbisEncInterface
{
public:
    virtual ~DllVorbisEnc() {};
    virtual int vorbis_encode_init(vorbis_info *vi, long channels, long rate, long max_bitrate, long nominal_bitrate, long min_bitrate)
        { return ::vorbis_encode_init(vi, channels, rate, max_bitrate, nominal_bitrate, min_bitrate); }
    virtual int vorbis_encode_init_vbr(vorbis_info *vi, long channels, long rate, float base_quality)
        { return ::vorbis_encode_init_vbr(vi, channels, rate, base_quality); }

    // DLL faking.
    virtual bool ResolveExports() { return true; }
    virtual bool Load() {
        CLog::Log(LOGDEBUG, "DllVorbisEnc: Using libvorbisenc system library");
        return true;
    }
    virtual void Unload() {}
};

#else

class DllVorbisEnc : public DllDynamic, DllVorbisEncInterface
{
  DECLARE_DLL_WRAPPER(DllVorbisEnc, DLL_PATH_VORBIS_ENC)
  DEFINE_METHOD6(int, vorbis_encode_init, (vorbis_info *p1, long p2, long p3, long p4, long p5, long p6))
  DEFINE_METHOD4(int, vorbis_encode_init_vbr, (vorbis_info *p1, long p2, long p3, float p4))
  BEGIN_METHOD_RESOLVE()
    RESOLVE_METHOD(vorbis_encode_init)
    RESOLVE_METHOD(vorbis_encode_init_vbr)
  END_METHOD_RESOLVE()
};

#endif
