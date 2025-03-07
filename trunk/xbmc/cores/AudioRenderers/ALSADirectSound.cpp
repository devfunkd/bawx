/*
* XBMC Media Center
* Copyright (c) 2002 d7o3g4q and RUNTiME
* Portions Copyright (c) by the authors of ffmpeg and xvid
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

#include "ALSADirectSound.h"

#ifdef HAS_ALSA

#include "AudioContext.h"
#include "FileSystem/SpecialProtocol.h"
#include "GUISettings.h"
#include "utils/SingleLock.h"
#include "utils/log.h"

#define CHECK_ALSA(l,s,e) if ((e)<0) CLog::Log(l,"%s - %s, alsa error: %d - %s",__FUNCTION__,s,e,snd_strerror(e));
#define CHECK_ALSA_RETURN(l,s,e) CHECK_ALSA((l),(s),(e)); if ((e)<0) return false;
using namespace std;

static CStdString EscapeDevice(const CStdString& device)
{
  CStdString result(device);
  result.Replace("'", "\\'");
  return result;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//***********************************************************************************************
CALSADirectSound::CALSADirectSound()
{
  m_pPlayHandle  = NULL;
  m_bIsAllocated = false;
}


// Be carefull - function is locked - should not call other locked functions 
// and should not exit in center.
bool CALSADirectSound::Initialize(IAudioCallback* pCallback, int iChannels, enum PCMChannels* channelMap, unsigned int uiSamplesPerSec, unsigned int uiBitsPerSample, bool bResample, const char* strAudioCodec, bool bIsMusic, bool bPassthrough,  bool bTimed, AudioMediaFormat audioMediaFormat)
{
  CSingleLock lock(m_lock);
  CStdString device, deviceuse;
  if (!bPassthrough)
  {
    if (g_guiSettings.GetString("audiooutput.audiodevice").Equals("custom"))
      device = g_guiSettings.GetString("audiooutput.customdevice");
    else
      device = g_guiSettings.GetString("audiooutput.audiodevice");
  } 
  else
  {
    if (g_guiSettings.GetString("audiooutput.passthroughdevice").Equals("custom"))
      device = g_guiSettings.GetString("audiooutput.custompassthrough");
    else
      device = g_guiSettings.GetString("audiooutput.passthroughdevice");
  }

  CLog::Log(LOGDEBUG,"CALSADirectSound::CALSADirectSound - Channels: %i - SampleRate: %i - SampleBit: %i - Resample %s - Codec %s - IsMusic %s - IsPassthrough %s - audioDevice: %s", iChannels, uiSamplesPerSec, uiBitsPerSample, bResample ? "true" : "false", strAudioCodec, bIsMusic ? "true" : "false", bPassthrough ? "true" : "false", device.c_str());

  if (iChannels == 0)
    iChannels = 2;

  bool bAudioOnAllSpeakers(false);
  g_audioContext.SetupSpeakerConfig(iChannels, bAudioOnAllSpeakers, bIsMusic);
  g_audioContext.SetActiveDevice(CAudioContext::DIRECTSOUND_DEVICE);

  m_pPlayHandle = NULL;
  m_bPause = false;
  m_bCanPause = false;
  m_bIsAllocated = false;
  m_uiChannels = iChannels;
  m_uiSamplesPerSec = uiSamplesPerSec;
  m_uiBitsPerSample = uiBitsPerSample;
  m_bPassthrough = bPassthrough;
  m_uiBytesPerSecond = uiSamplesPerSec * (uiBitsPerSample / 8) * iChannels;

  m_nCurrentVolume = g_stSettings.m_nVolumeLevel;
  if (!m_bPassthrough)
     m_amp.SetVolume(m_nCurrentVolume);


  // Boxee - zvi 
  // I want big frame count because we get statics and buffer underrun because of long
  // contex-switches.
  snd_pcm_uframes_t dwFrameCount = 512*4;
  unsigned int      dwNumPackets = 16;
  snd_pcm_uframes_t dwBufferSize = 0;

  snd_pcm_hw_params_t *hw_params=NULL;
  snd_pcm_sw_params_t *sw_params=NULL;

  /* Open the device */
  int nErr;

  /* if this is first access to audio, global sound config might not be loaded */
  if(!snd_config)
    snd_config_update();

  snd_config_t *config = snd_config;
  deviceuse = device;

  nErr = snd_config_copy(&config, snd_config);
  CHECK_ALSA_RETURN(LOGERROR,"config_copy",nErr);

  if(m_bPassthrough)
  {
      /* http://www.alsa-project.org/alsa-doc/alsa-lib/group___digital___audio___interface.html */
      deviceuse += ":AES0=0x6";
      deviceuse += ",AES1=0x82";
      deviceuse += ",AES2=0x0";
      if(uiSamplesPerSec == 48000)
        deviceuse += ",AES3=0x2";
      else if(uiSamplesPerSec == 44100)
        deviceuse += ",AES3=0x0";
      else if(uiSamplesPerSec == 32000)
        deviceuse += ",AES3=0x3";
      else
        deviceuse += ",AES3=0x1"; /* just a guess, maybe works for 96k */
    }
  else
  {
    if(deviceuse == "hdmi"
    || deviceuse == "iec958"
    || deviceuse == "spdif")
      deviceuse = "plug:" + deviceuse;

    if(g_guiSettings.GetBool("audiooutput.downmixmultichannel"))
    {
      if(iChannels == 6)
        deviceuse = "xbmc_51to2:'" + EscapeDevice(deviceuse) + "'";
      else if(iChannels == 5)
        deviceuse = "xbmc_50to2:'" + EscapeDevice(deviceuse) + "'";
    }
    else
    {
      if(deviceuse == "default")
      {
        if(iChannels == 6)
          deviceuse = "surround51";
        else if(iChannels == 5)
          deviceuse = "surround50";
        else if(iChannels == 4)
          deviceuse = "surround40";
      }
    }

    // setup channel mapping to linux default
    if (strstr(strAudioCodec, "AAC"))
    {
      if(iChannels == 6)
        deviceuse = "xbmc_aac51:'" + EscapeDevice(deviceuse) + "'";
      else if(iChannels == 5)
        deviceuse = "xbmc_aac50:'" + EscapeDevice(deviceuse) + "'";
    }
    else if (strstr(strAudioCodec, "DMO") || strstr(strAudioCodec, "FLAC") || strstr(strAudioCodec, "PCM"))
    {
      if(iChannels == 6)
        deviceuse = "xbmc_win51:'" + EscapeDevice(deviceuse) + "'";
      else if(iChannels == 5)
        deviceuse = "xbmc_win50:'" + EscapeDevice(deviceuse) + "'";
    }
    else if (strstr(strAudioCodec, "OggVorbis"))
    {
      if(iChannels == 6)
        deviceuse = "xbmc_ogg51:'" + EscapeDevice(deviceuse) + "'";
      else if(iChannels == 5)
        deviceuse = "xbmc_ogg50:'" + EscapeDevice(deviceuse) + "'";
    }


    if(deviceuse != device)
    {
      snd_input_t* input;
      nErr = snd_input_stdio_open(&input, _P("special://xbmc/system/asound.conf").c_str(), "r");
      if(nErr >= 0)
      {
        nErr = snd_config_load(config, input);
        CHECK_ALSA_RETURN(LOGERROR,"config_load", nErr);

        snd_input_close(input);
        CHECK_ALSA_RETURN(LOGERROR,"input_close", nErr);
      }
      else
      {
        CLog::Log(LOGWARNING, "%s - Unable to load alsa configuration \"%s\" for device \"%s\" - %s", __FUNCTION__, "special://xbmc/system/asound.conf", deviceuse.c_str(), snd_strerror(nErr));
        deviceuse = device;
      }
    }
  }

  CLog::Log(LOGDEBUG, "%s - using alsa device %s", __FUNCTION__, deviceuse.c_str());

  nErr = snd_pcm_open_lconf(&m_pPlayHandle, deviceuse.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK, config);

  if(nErr == -EBUSY)
  {
    // this could happen if we are in the middle of a resolution switch sometimes
    CLog::Log(LOGERROR, "%s - device %s busy retrying...", __FUNCTION__, deviceuse.c_str());
    if(m_pPlayHandle)
    {
      snd_pcm_close(m_pPlayHandle);
      m_pPlayHandle = NULL;
    }
    Sleep(200);
    nErr = snd_pcm_open_lconf(&m_pPlayHandle, deviceuse.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK, config);
  }

  if(nErr < 0 && deviceuse != device)
  {
    CLog::Log(LOGERROR, "%s - failed to open custom device %s, retry with default %s", __FUNCTION__, deviceuse.c_str(), device.c_str());
    if(m_pPlayHandle)
    {
      snd_pcm_close(m_pPlayHandle);
      m_pPlayHandle = NULL;
    }
    nErr = snd_pcm_open_lconf(&m_pPlayHandle, device.c_str(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK, config);

  }

  CHECK_ALSA_RETURN(LOGERROR,"pcm_open_lconf",nErr);

  snd_config_delete(config);

  /* Allocate Hardware Parameters structures and fills it with config space for PCM */
  snd_pcm_hw_params_malloc(&hw_params);

  /* Allocate Software Parameters structures and fills it with config space for PCM */
  snd_pcm_sw_params_malloc(&sw_params);

  nErr = snd_pcm_hw_params_any(m_pPlayHandle, hw_params);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_any",nErr);

  nErr = snd_pcm_hw_params_set_access(m_pPlayHandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_access",nErr);

  // always use 16 bit samples
  nErr = snd_pcm_hw_params_set_format(m_pPlayHandle, hw_params, SND_PCM_FORMAT_S16_LE);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_format",nErr);

  nErr = snd_pcm_hw_params_set_rate_near(m_pPlayHandle, hw_params, &m_uiSamplesPerSec, NULL);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_rate",nErr);

  nErr = snd_pcm_hw_params_set_channels(m_pPlayHandle, hw_params, iChannels);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_channels",nErr);

  nErr = snd_pcm_hw_params_set_period_size_near(m_pPlayHandle, hw_params, &dwFrameCount, NULL);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_period_size",nErr);

  nErr = snd_pcm_hw_params_set_periods_near(m_pPlayHandle, hw_params, &dwNumPackets, NULL);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_set_periods",nErr);

  nErr = snd_pcm_hw_params_get_buffer_size(hw_params, &dwBufferSize);
  CHECK_ALSA_RETURN(LOGERROR,"hw_params_get_buffer_size",nErr);

  /* Assign them to the playback handle and free the parameters structure */
  nErr = snd_pcm_hw_params(m_pPlayHandle, hw_params);
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_hw_params",nErr);

  nErr = snd_pcm_sw_params_current(m_pPlayHandle, sw_params);
  CHECK_ALSA_RETURN(LOGERROR,"sw_params_current",nErr);

  nErr = snd_pcm_sw_params_set_start_threshold(m_pPlayHandle, sw_params, dwFrameCount);
  CHECK_ALSA_RETURN(LOGERROR,"sw_params_set_start_threshold",nErr);

  snd_pcm_uframes_t boundary;
  nErr = snd_pcm_sw_params_get_boundary( sw_params, &boundary );
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_sw_params_get_boundary",nErr);

  nErr = snd_pcm_sw_params_set_silence_threshold(m_pPlayHandle, sw_params, 0 );
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_sw_params_set_silence_threshold",nErr);

  nErr = snd_pcm_sw_params_set_silence_size( m_pPlayHandle, sw_params, boundary );
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_sw_params_set_silence_size",nErr);

  nErr = snd_pcm_sw_params(m_pPlayHandle, sw_params);
  CHECK_ALSA_RETURN(LOGERROR,"snd_pcm_sw_params",nErr);

  m_bCanPause    = !!snd_pcm_hw_params_can_pause(hw_params);
  m_dwPacketSize = snd_pcm_frames_to_bytes(m_pPlayHandle, dwFrameCount);
  m_dwNumPackets = dwNumPackets;
  m_uiBufferSize = snd_pcm_frames_to_bytes(m_pPlayHandle, dwBufferSize);

  CLog::Log(LOGDEBUG, "CALSADirectSound::Initialize - packet size:%u, packet count:%u, buffer size:%u"
                    , (unsigned int)m_dwPacketSize, m_dwNumPackets, (unsigned int)dwBufferSize);

  if(m_uiSamplesPerSec != uiSamplesPerSec)
    CLog::Log(LOGWARNING, "CALSADirectSound::CALSADirectSound - requested samplerate (%d) not supported by hardware, using %d instead", uiSamplesPerSec, m_uiSamplesPerSec);


  nErr = snd_pcm_prepare (m_pPlayHandle);
  CHECK_ALSA(LOGERROR,"snd_pcm_prepare",nErr);

  snd_pcm_hw_params_free (hw_params);
  snd_pcm_sw_params_free (sw_params);
  
  m_bIsAllocated = true;
  return true;
}

//***********************************************************************************************
CALSADirectSound::~CALSADirectSound()
{
  Deinitialize();
}


//***********************************************************************************************
bool CALSADirectSound::Deinitialize()
{
  CSingleLock lock(m_lock);
  m_bIsAllocated = false;
  if (m_pPlayHandle)
  {
    snd_pcm_drop(m_pPlayHandle);
    snd_pcm_close(m_pPlayHandle);
  }

  m_pPlayHandle=NULL;
  g_audioContext.SetActiveDevice(CAudioContext::DEFAULT_DEVICE);
  return true;
}

void CALSADirectSound::Flush() {
  CSingleLock lock(m_lock);
  FlushUnlocked();
}


void CALSADirectSound::FlushUnlocked() {
  if (!m_bIsAllocated)
    return;
  int nErr = snd_pcm_drop(m_pPlayHandle);
  CHECK_ALSA(LOGERROR,"flush-drop",nErr);
  nErr = snd_pcm_prepare(m_pPlayHandle);
  CHECK_ALSA(LOGERROR,"flush-prepare",nErr);
}

//***********************************************************************************************
bool CALSADirectSound::Pause()
{
  if (!m_bIsAllocated)
     return false;
  CSingleLock lock(m_lock);
  if (m_bPause) return true;
  m_bPause = true;

  if(m_bCanPause)
  {
    int nErr = snd_pcm_pause(m_pPlayHandle,1); // this is not supported on all devices.     
    CHECK_ALSA(LOGERROR,"pcm_pause",nErr);
    if(nErr<0)
      m_bCanPause = false;
  }


  // Because of ALSA bug we reset here every time -Zvisha.
  FlushUnlocked();

  return true;
}

//***********************************************************************************************
bool CALSADirectSound::Resume()
{
  if (!m_bIsAllocated)
     return -1;
  CSingleLock lock(m_lock);
  
  // Because of ALSA bug we reset here every time -Zvisha.
  FlushUnlocked();
  if(m_bCanPause && m_bPause)
    int ret = snd_pcm_pause(m_pPlayHandle,0);
  m_bPause = false;

  return true;
}

//***********************************************************************************************
bool CALSADirectSound::Stop()
{
  if (!m_bIsAllocated)
     return -1;
  CSingleLock lock(m_lock);
  FlushUnlocked();
  m_bPause = false;
  return true;
}

//***********************************************************************************************
long CALSADirectSound::GetCurrentVolume() const
{
  return m_nCurrentVolume;
}

//***********************************************************************************************
void CALSADirectSound::Mute(bool bMute)
{
  if (!m_bIsAllocated) 
    return;

  if (bMute)
    SetCurrentVolume(VOLUME_MINIMUM);
  else
    SetCurrentVolume(m_nCurrentVolume);

}

//***********************************************************************************************
bool CALSADirectSound::SetCurrentVolume(long nVolume)
{
  if (!m_bIsAllocated || m_bPassthrough) return -1;
  m_nCurrentVolume = nVolume;
  m_amp.SetVolume(nVolume);
  return true;
}


//***********************************************************************************************
unsigned int CALSADirectSound::GetSpace()
{
  if (!m_bIsAllocated) return 0;
  CSingleLock lock(m_lock);
  int nSpace = snd_pcm_avail_update(m_pPlayHandle);
  if (nSpace == 0) 
  {
    snd_pcm_state_t state = snd_pcm_state(m_pPlayHandle);
    if(state != SND_PCM_STATE_RUNNING && !m_bPause)
    {
      DBG(DAUDIO,5,"buffer underun");
      CLog::Log(LOGWARNING,"CALSADirectSound::GetSpace - buffer XRUN (%d)", state);
      FlushUnlocked();
    }
  }
  if (nSpace < 0) {
     DBG(DAUDIO,5,"CALSADirectSound::GetSpace - get space failed. err: %d (%s)", nSpace, snd_strerror(nSpace));
     CLog::Log(LOGWARNING,"CALSADirectSound::GetSpace - get space failed. err: %d (%s)", nSpace, snd_strerror(nSpace));
     nSpace = 0;
     FlushUnlocked();
  }
  unsigned int x = snd_pcm_frames_to_bytes(m_pPlayHandle, nSpace);
  return x;
}

//***********************************************************************************************
unsigned int CALSADirectSound::AddPackets(const void* data, unsigned int len, double pts, double duration)
{
  CSingleLock lock(m_lock);
  DBG(DAUDIO,5,"Add %d bytes", len);
  // FlushUnlocked();
  // snd_pcm_drain(m_pPlayHandle);

  unsigned int r = 0;
  do {
    if (!m_bIsAllocated) {
      CLog::Log(LOGERROR,"CALSADirectSound::AddPackets - sanity failed. no valid play handle!");
      r = len; 
      break;
    }
    // if we are paused we don't accept any data as pause doesn't always
    // work, and then playback would start again
    if(m_bPause) {
      r = 0;
      break;
    }

    const unsigned char *pcmPtr = (const unsigned char *)data;
    int framesToWrite;

    framesToWrite  = std::min(GetSpace(), len);
    framesToWrite /= m_dwPacketSize;
    framesToWrite *= m_dwPacketSize;
    framesToWrite  = snd_pcm_bytes_to_frames(m_pPlayHandle, framesToWrite);
 
    if(framesToWrite == 0) { 
      r = 0;
      break;
    }

    // handle volume de-amp 
    if (!m_bPassthrough)
      m_amp.DeAmplify((short *)pcmPtr, framesToWrite * m_uiChannels);

    int writeResult = snd_pcm_writei(m_pPlayHandle, pcmPtr, framesToWrite);
    if (  writeResult == -EPIPE  ) {
      DBG(DAUDIO,5,"EPIPE");
      snd_pcm_recover(m_pPlayHandle, writeResult, 1);
      writeResult = snd_pcm_writei(m_pPlayHandle, pcmPtr, framesToWrite);
      r = 0;
      break;
    }

    if (writeResult != framesToWrite) {
      CLog::Log(LOGERROR, "CALSADirectSound::AddPackets - failed to write %d frames. "
              "bad write (err: %d) - %s",
              framesToWrite, writeResult, snd_strerror(writeResult));
      DBG(DAUDIO,5,"bug");
      FlushUnlocked();
    }

    if (writeResult>0)
      pcmPtr += snd_pcm_frames_to_bytes(m_pPlayHandle, writeResult);

    r = writeResult * (m_uiBitsPerSample / 8) * m_uiChannels;
  } while (0);
  return r;
}

//***********************************************************************************************
float CALSADirectSound::GetDelay()
{
  if (!m_bIsAllocated) {
    return 0.0;
  }
  CSingleLock lock(m_lock);
  snd_pcm_sframes_t frames = 0;

  int nErr = snd_pcm_delay(m_pPlayHandle, &frames);
  CHECK_ALSA(LOGERROR,"snd_pcm_delay",nErr); 
  if (nErr < 0) {
    frames = 0;
    DBG(DAUDIO,5,"err");
    FlushUnlocked();
  }

  if (frames < 0) {
#if SND_LIB_VERSION >= 0x000901 /* snd_pcm_forward() exists since 0.9.0rc8 */
    snd_pcm_forward(m_pPlayHandle, -frames);
#endif
    frames = 0;
  }
  return (double)frames / m_uiSamplesPerSec;
}

float CALSADirectSound::GetCacheTime()
{
  return (float)(m_uiBufferSize - GetSpace()) / (float)m_uiBytesPerSecond;
}

//***********************************************************************************************
unsigned int CALSADirectSound::GetChunkLen()
{
  return m_dwPacketSize;
}
//***********************************************************************************************
int CALSADirectSound::SetPlaySpeed(int iSpeed)
{
  return 0;
}

void CALSADirectSound::RegisterAudioCallback(IAudioCallback *pCallback)
{
  m_pCallback = pCallback;
}

void CALSADirectSound::UnRegisterAudioCallback()
{
  m_pCallback = NULL;
}

void CALSADirectSound::WaitCompletion()
{
  if (!m_bIsAllocated || m_bPause)
    return;
  int retval = snd_pcm_drain(m_pPlayHandle);
  CSingleLock lock(m_lock);

  // Boxee fix - while calling snd_pcm_drain deinitialized or pause function could have happend
  // if it is the case we dont need to do any thing.
  // lock m_lock before call snd_pcm_drain slows the significantly
  if (!m_bIsAllocated || m_bPause)
    return;
  FlushUnlocked();
  snd_pcm_wait(m_pPlayHandle, -1);
}

void CALSADirectSound::SwitchChannels(int iAudioStream, bool bAudioOnAllSpeakers)
{
    return ;
}

bool CALSADirectSound::SoundDeviceExists(const CStdString& device)
{
  void **hints, **n;
  char *name;
  CStdString strName;
  bool retval = false;

  if (snd_device_name_hint(-1, "pcm", &hints) == 0)
  {
    for (n = hints; *n; n++)
    {
      if ((name = snd_device_name_get_hint(*n, "NAME")) != NULL)
      {
        strName = name;
        if (strName.find(device) != string::npos)
        {
          retval = true;
          break;
        }
        free(name);
      }
    }
    snd_device_name_free_hint(hints);
  }
  return retval;
}

void CALSADirectSound::GetSoundCards(std::vector<CStdString>& vSoundCards)
{
  void **hints, **n;
  char *name;

  vSoundCards.clear();

  if (snd_device_name_hint(-1, "pcm", &hints) < 0)
    return;

  n = hints;
  while (*n != NULL)
  {
    name = snd_device_name_get_hint(*n, "NAME");

    if (name != NULL)
    {
      vSoundCards.push_back(name);
      free(name);
    }

    n++;
  }

  snd_device_name_free_hint(hints);
}
#endif
