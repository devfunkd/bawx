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

#include "system.h"
#include "AudioRendererFactory.h"
#include "NullDirectSound.h"
#include "GUISettings.h"

#ifdef _WIN32
#include "Win32DirectSound.h"
#include "Win32WASAPI.h"
#endif
#ifdef __APPLE__
#include "CoreAudioRenderer.h"
#elif defined(HAS_INTEL_SMD)
#include "IntelSMDAudioRenderer.h"
#elif defined(HAS_ALSA)
#include "ALSADirectSound.h"
#endif

#define ReturnOnValidInitialize()          \
{                                          \
  if (audioSink->Initialize(pCallback, iChannels, channelMap, uiSamplesPerSec, uiBitsPerSample, bResample, strAudioCodec, bIsMusic, bPassthrough, bTimed, audioMediaFormat))  \
    return audioSink;                      \
  else                                     \
  {                                        \
    audioSink->Deinitialize();             \
    delete audioSink;                      \
    audioSink = 0;                         \
  }                                        \
}\

IAudioRenderer* CAudioRendererFactory::Create(IAudioCallback* pCallback, int iChannels, enum PCMChannels* channelMap, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, bool bResample, const char* strAudioCodec, bool bIsMusic, bool bPassthrough, bool bTimed, AudioMediaFormat audioMediaFormat)
{
  IAudioRenderer *audioSink = CreateAudioRenderer(pCallback, iChannels, channelMap, uiSamplesPerSec, uiBitsPerSample, bResample, strAudioCodec, bIsMusic, bPassthrough, bTimed, audioMediaFormat);

  return audioSink;
}

IAudioRenderer* CAudioRendererFactory::CreateAudioRenderer(IAudioCallback* pCallback, int iChannels, enum PCMChannels* channelMap, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, bool bResample, const char* strAudioCodec, bool bIsMusic, bool bPassthrough, bool bTimed, AudioMediaFormat audioMediaFormat)
{
  IAudioRenderer* audioSink = NULL;

/* in case none in the first pass was able to be created, fall back to os specific */

#ifdef WIN32
/*  if (bPassthrough)
  {
    audioSink = new CWin32WASAPI();
    ReturnOnValidInitialize();
  }
  else
  {*/
  audioSink = new CWin32DirectSound();
  ReturnOnValidInitialize();
#elif defined(__APPLE__)
  audioSink = new CCoreAudioRenderer();
  ReturnOnValidInitialize();
#elif defined(HAS_INTEL_SMD)
  audioSink = new CIntelSMDAudioRenderer();
  ReturnOnValidInitialize();
#elif defined(HAS_ALSA)
  int requestedLibrary = g_guiSettings.GetInt("audiooutput.library");
  if (requestedLibrary == AUDIO_LIBRARY_ALSA)
  {
    audioSink = new CALSADirectSound();
    ReturnOnValidInitialize();
  }
#endif

  audioSink = new CNullDirectSound();
  audioSink->Initialize(pCallback, iChannels, channelMap, uiSamplesPerSec, uiBitsPerSample, bResample, strAudioCodec, bIsMusic, bPassthrough, bTimed, audioMediaFormat);
  return audioSink;
}
